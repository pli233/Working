#include <memory.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include "page.h"
#include "buf.h"




#define ASSERT(c)  { if (!(c)) { \
		       cerr << "At line " << __LINE__ << ":" << endl << "  "; \
                       cerr << "This condition should hold: " #c << endl; \
                       exit(1); \
		     } \
                   }

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------


BufMgr::BufMgr(const int bufs)
{
    numBufs = bufs;  // Initialize the number of buffers

    bufTable = new BufDesc[bufs]; // Create the hashtable
    memset(bufTable, 0, bufs * sizeof(BufDesc));

    //Initialize buffer table
    // buf table is the
    // assign frame number and set valid
    for (int i = 0; i < bufs; i++) 
    {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2))*2)/2)+1;
    hashTable = new BufHashTbl (htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}

//----------------------------------------
// Destructor of the class BufMgr
//----------------------------------------
BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) 
    {
        BufDesc* tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i]));
        }
    }

    delete [] bufTable;
    delete [] bufPool;
}



/*
 * Allocates a free frame in bufPool using the clock algorithm
 *
 * 1) Use Clock algorithm to determine which page to evict
 * 2) If a page evicted is dirty, write it back to disk
 * 3) Returns BUFFEREXCEEDED if all buffer frames are pinned
 * 4) Returns UNIXERR if the call to the I/O layer returned an error when a dirty page was being written to disk and OK otherwise
 * 5) Make sure that if the buffer frame allocated has a valid page in it,
 * that you remove the appropriate entry from the hash table.
 * 6) This private method will get called by the readPage() and allocPage() methods described below.
 * */


const Status BufMgr::allocBuf(int & frame) 
{
    bool foundUnpinned = false; // To track if we have any unpinned frame at all.
    bool foundRefBitReset = false; // To track if we've reset any reference bits.

// Start from the current clock hand position.
    int initialPointer = clockHand;

    do {
        BufDesc* currentFrame = &bufTable[clockHand];

        // If frame is unpinned, note that we found an unpinned frame.
        if (currentFrame->pinCnt == 0) {
            foundUnpinned = true;

            // If frame is also not recently accessed, it's our candidate.
            if (!currentFrame->refbit) {
                // (The same code to handle dirty page write-back and removal from hash table)

                if (currentFrame->dirty) {
                    Status writeStatus = currentFrame->file->writePage(currentFrame->pageNo, &bufPool[clockHand]);
                    if (writeStatus != OK) {
                        return UNIXERR; // 写回磁盘失败。
                    }
                    currentFrame->dirty = false; // 重置脏标志
                    bufStats.diskwrites++; // 更新磁盘写入统计
                }

                if (currentFrame->valid) {
                    hashTable->remove(currentFrame->file, currentFrame->pageNo);
                    currentFrame->valid = false; // 标记为无效，以避免后续的错误查找。
                }

                frame = clockHand; // Assign this frame.
                advanceClock(); // Move clock hand for the next allocation.
                return OK; // Successfully allocated a frame.
            } else {
                // Current frame was recently accessed; give it a second chance.
                currentFrame->refbit = false;
                foundRefBitReset = true; // We've reset at least one refbit.
            }
        }

        advanceClock(); // Move to the next frame.

        // If we've returned to the start and haven't reset any refbit during this pass, exit.
    } while (clockHand != initialPointer || foundRefBitReset);

// After the loop, decide based on whether we found any unpinned frame.
    if (foundUnpinned) {
        return OK; // Indicates buffer pool is fully utilized with pinned pages.
    } else {
        // This condition indicates every frame was pinned throughout,
        // which also leads to buffer pool being considered full.
        return BUFFEREXCEEDED;
    }
}

	
const Status BufMgr::readPage(File* file, const int PageNo, Page*& page)
{
    int frameNo;

    // First, check if the page is already in the buffer pool
    Status status = hashTable->lookup(file, PageNo, frameNo);

    if (status == OK) {
        // Case 2: Page is in the buffer pool
        BufDesc* bufDesc = &bufTable[frameNo];
        bufDesc->refbit = true; // Set the appropriate refbit
        bufDesc->pinCnt++; // Increment the pinCnt for the page
        page = &bufPool[frameNo]; // Return a pointer to the frame containing the page
    } else {
        // Case 1: Page is not in the buffer pool
        status = allocBuf(frameNo); // Allocate a buffer frame
        if (status != OK) {
            return status; // Could be BUFFEREXCEEDED or UNIXERR
        }

        // Read the page from disk into the buffer pool frame
        status = file->readPage(PageNo, &bufPool[frameNo]);
        if (status != OK) {
            return UNIXERR; // Assuming any error here is a Unix error
        }

        // Insert the page into the hash table
        status = hashTable->insert(file, PageNo, frameNo);
        if (status != OK) {
            return HASHTBLERROR; // If inserting into the hash table fails
        }

        // Set up the frame properly
        bufTable[frameNo].Set(file, PageNo); // This will leave the pinCnt for the page set to 1
        page = &bufPool[frameNo]; // Return a pointer to the frame containing the page
    }

    return OK; // If no errors occurred
}


const Status BufMgr::unPinPage(File* file, const int PageNo, 
			       const bool dirty) 
{
    int frameNo;
    // Step 1: Look up the frame in the hash table
    Status status = hashTable->lookup(file, PageNo, frameNo);
    if (status != OK) {
        return HASHNOTFOUND; // The page is not found in the buffer pool
    }

    // Reference the BufDesc for the found frame
    BufDesc* bufDesc = &bufTable[frameNo];

    // Step 2: Check the pin count
    if (bufDesc->pinCnt <= 0) {
        // Pin count should not be less than 0; indicate an error if it is
        return PAGENOTPINNED; // The page is not pinned
    }

    // Step 3: Decrement the pin count and set dirty bit if necessary
    bufDesc->pinCnt--;
    if (dirty) {
        bufDesc->dirty = true; // Mark the page as dirty
    }

    // Step 4: Return OK on successful operation
    return OK;
}

const Status BufMgr::allocPage(File* file, int& pageNo, Page*& page) 
{
    Status status;
    int frameNo;

    // Step 1: Allocate an empty page in the file
    status = file->allocatePage(pageNo);
    if (status != OK) {
        return status; // Could potentially be UNIXERR if there's a file-level error
    }

    // Step 2: Obtain a buffer pool frame
    status = allocBuf(frameNo);
    if (status != OK) {
        // If no frame could be allocated, it's necessary to deallocate the page in the file
        file->disposePage(pageNo);
        return status; // Could be BUFFEREXCEEDED if all frames are pinned
    }

    // Step 4: Setup the buffer frame properly
    bufTable[frameNo].Set(file, pageNo);

    // Get a pointer to the allocated frame in the buffer pool
    page = &bufPool[frameNo];

    // Step 3: Insert an entry into the hash table
    status = hashTable->insert(file, pageNo, frameNo);
    if (status != OK) {
        // Cleanup if the hash table insertion fails
        bufTable[frameNo].Clear(); // Clear the buffer descriptor to make it available again
        file->disposePage(pageNo); // Deallocate the page from the file
        return HASHTBLERROR;
    }

    return OK; // Successful allocation and setup, returns the page number and frame pointer to the caller
}

const Status BufMgr::disposePage(File* file, const int pageNo) 
{
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK)
    {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File* file) 
{
  Status status;

  for (int i = 0; i < numBufs; i++) {
    BufDesc* tmpbuf = &(bufTable[i]);
    if (tmpbuf->valid == true && tmpbuf->file == file) {

      if (tmpbuf->pinCnt > 0)
	  return PAGEPINNED;

      if (tmpbuf->dirty == true) {
#ifdef DEBUGBUF
	cout << "flushing page " << tmpbuf->pageNo
             << " from frame " << i << endl;
#endif
	if ((status = tmpbuf->file->writePage(tmpbuf->pageNo,
					      &(bufPool[i]))) != OK)
	  return status;

	tmpbuf->dirty = false;
      }

      hashTable->remove(file,tmpbuf->pageNo);

      tmpbuf->file = NULL;
      tmpbuf->pageNo = -1;
      tmpbuf->valid = false;
    }

    else if (tmpbuf->valid == false && tmpbuf->file == file)
      return BADBUFFER;
  }
  
  return OK;
}


void BufMgr::printSelf(void) 
{
    BufDesc* tmpbuf;
  
    cout << endl << "Print buffer...\n";
    for (int i=0; i<numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char*)(&bufPool[i]) 
             << "\tpinCnt: " << tmpbuf->pinCnt;
    
        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


