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


BufMgr::BufMgr(const int bufs) {
    numBufs = bufs;  // Initialize the number of buffers

    bufTable = new BufDesc[bufs]; // Create the hashtable
    memset(bufTable, 0, bufs * sizeof(BufDesc));

    //Initialize buffer table
    // buf table is the
    // assign frame number and set valid
    for (int i = 0; i < bufs; i++) {
        bufTable[i].frameNo = i;
        bufTable[i].valid = false;
    }

    bufPool = new Page[bufs];
    memset(bufPool, 0, bufs * sizeof(Page));

    int htsize = ((((int) (bufs * 1.2)) * 2) / 2) + 1;
    hashTable = new BufHashTbl(htsize);  // allocate the buffer hash table

    clockHand = bufs - 1;
}

//----------------------------------------
// Destructor of the class BufMgr
//----------------------------------------
BufMgr::~BufMgr() {

    // flush out all unwritten pages
    for (int i = 0; i < numBufs; i++) {
        BufDesc *tmpbuf = &bufTable[i];
        if (tmpbuf->valid == true && tmpbuf->dirty == true) {

#ifdef DEBUGBUF
            cout << "flushing page " << tmpbuf->pageNo
                 << " from frame " << i << endl;
#endif

            tmpbuf->file->writePage(tmpbuf->pageNo, &(bufPool[i])); //how we write back to disk
        }
    }

    delete[] bufTable;
    delete[] bufPool;
}


/**
 * Allocates a free frame in bufPool using the clock algorithm.
 * This private method will get called by the readPage() and allocPage() methods.
 *
 * 1) Uses the Clock algorithm to determine which page to evict.
 * 2) If a page evicted is dirty, it is written back to disk.
 * 3) Ensures that if the buffer frame allocated has a valid page in it,
 *    the appropriate entry is removed from the hash table.
 *
 * \return BUFFEREXCEEDED if all buffer frames are pinned.
 * \return UNIXERR if the call to the I/O layer returned an error when a dirty page was being written to disk.
 * \return OK otherwise.
 * \param[out] frame The clockhand value is assigned to user-provided frame number variable.
 */


const Status BufMgr::allocBuf(int &frame) {
    // Using Clock Algorithm to visit buffer pool

    // 1. Initialize variable for clock algorithm
    unsigned int initialPointer = clockHand; // Record the start position for checking loop pass all the frame later
    bool foundUnpinButRef = false; // Track if there is any unrefereed frame in buffer pool

    // 2. Start search
    do {
        BufDesc *currentFrame = &bufTable[clockHand];

        // 2.1 Check if frame is valid
        // If a frame is not valid, it is a fresh new frame free to insert
        if (!currentFrame->valid) {
            frame = clockHand; // Use frame if not valid.
            return OK; //return correspond status code and finish program
        }


        // 2.2 Check if the page is pinned
        // If a frame is pinned, it will never be replaced since some program is using it
        if (currentFrame->pinCnt > 0) {
            advanceClock(); // Advance clock pointer to next frame.
            continue; // Skip if pinned and continue searching.
        }

        //2.3 Check if refBit is set
        // If the bit had been set, the rame has been referenced "recently" and is not replaced, clear the bit and continue loop
        // On the other hand, if the refbit is false, the page is selected for replacement

        if (currentFrame->refbit) {
            currentFrame->refbit = false; // Clear refBit and continue searching.
            foundUnpinButRef = true; //telling the program there is some frame unpinned but referred
        } else {

            // If we reached here, frame is valid, unpinned, and refBit is not set
            // The frame is a candidate for replacement at this point (unpinned and refBit is not set)
            // Process with replace logic ......

            //3. Check if the frame is dirty
            // If the selected buffer frame is dirty, the page currently occupying the frame is written back to disk
            // Otherwise the frame is just cleared (using the clear() function) and a new page from disk is read in to that location

            //3.1 Dirty
            if (currentFrame->dirty) {
                // Flush the page to disk
                Status writeStatus = currentFrame->file->writePage(currentFrame->pageNo, &bufPool[clockHand]);
                if (writeStatus != OK) {
                    return UNIXERR; // Writing back to disk failed, return with pre-determined error code
                }
                currentFrame->dirty = false; // Reset dirty flag
                bufStats.diskwrites++; // Update disk write statistics
            }


            hashTable->remove(currentFrame->file,
                              currentFrame->pageNo); // Remove page from the hash table if it's present.

            currentFrame->Clear();  // Clear the frame, no need to set valid bit to false, it is done by the clear function

            frame = clockHand; // Assign this frame for replacement
            return OK; // Successfully allocated a frame
        }

        //4. Advance clock pointer to next frame after processing current frame.
        advanceClock();
    } while (clockHand != initialPointer ||
             foundUnpinButRef); // 5. This logic only fail if we loop through all frames and no unpinned but referred frame detected

    //6. If we've completed a full cycle without finding an unpinned but referred frame,
    // no more reason for search since no frame deserve second chance
    // Buffer pool is fully utilized with pinned pages.

    return BUFFEREXCEEDED; // Indicates buffer max with pre-determined error code
}


/**
 * Attempts to read a specified page from a file into the buffer pool. If the page is already present in the buffer pool,
 * it updates the metadata of the frame containing the page. If the page is not in the buffer pool, it loads the page from
 * disk into a free frame in the buffer pool.
 *
 * The function first checks the hash table to determine if the page is already loaded. If the page is found, it increments
 * the pin count and sets the reference bit to true, indicating that the page is being accessed. If the page is not found,
 * it allocates a new buffer frame, reads the page from disk into this frame, inserts the page into the hash table, and
 * sets up the frame's metadata.
 *
 * \param[in] file A pointer to the File object representing the file containing the page.
 * \param[in] PageNo The page number within the file to be read.
 * \param[out] page A pointer to a Page pointer (double pointer), which will point to the page frame containing the requested page upon success.
 * \return OK if the page was successfully read into the buffer pool or was already present.
 * \return BUFFEREXCEEDED if there are no free frames available in the buffer pool.
 * \return UNIXERR if there was an error reading the page from disk.
 * \return HASHTBLERROR if inserting the page into the hash table failed.
 */

const Status BufMgr::readPage(File *file, const int PageNo, Page *&page) {
    int frameNo;

    // 1. Use hashtable to check if the page is already in the buffer pool
    Status status = hashTable->lookup(file, PageNo, frameNo);

    // 2. Page is in the hashTable and buffer pool, we can read buf pool to load it
    if (status == OK) {
        BufDesc *bufDesc = &bufTable[frameNo];
        bufDesc->refbit = true; // Set the appropriate refbit
        bufDesc->pinCnt++; // Increment the pinCnt for the page
        page = &bufPool[frameNo]; // Return a pointer to the frame containing the page
    } else {
        // 3. Page is not in the buffer pool

        // 3.1 Try to Allocate a buffer frame
        status = allocBuf(frameNo);
        if (status != OK) {
            return status; // Could be BUFFEREXCEEDED or UNIXERR
        }

        // Successfully allocate a page
        // 3.2 Read the page from disk into the buffer pool frame
        status = file->readPage(PageNo, &bufPool[frameNo]);
        if (status != OK) {
            return UNIXERR; // Assuming any error here is a Unix error
        }

        // 3.3 Insert the page into the hash table
        status = hashTable->insert(file, PageNo, frameNo);
        if (status != OK) {
            return HASHTBLERROR; // If inserting into the hash table fails
        }

        // 3.4 Set up the frame properly
        bufTable[frameNo].Set(file, PageNo); // This will leave the pinCnt for the page set to 1
        page = &bufPool[frameNo]; // Return a pointer to the frame containing the page
    }

    return OK; // If no errors occurred
}



/**
 * Decrements the pin count of a specified page in the buffer pool and optionally marks it as dirty. The function looks up
 * the page in the hash table to find its corresponding frame. If found, it checks the pin count to ensure it is not already
 * zero (indicating an error if it is) and then decrements the pin count. If the 'dirty' parameter is true, the function also
 * sets the dirty bit for the page, indicating that it has been modified and needs to be written back to disk upon eviction.
 *
 * \param[in] file A pointer to the File object representing the file containing the page.
 * \param[in] PageNo The page number within the file to be unpinned.
 * \param[in] dirty A boolean flag indicating whether the page has been modified.
 * \return OK if the operation is successful.
 * \return HASHNOTFOUND if the page is not found in the hash table, indicating it is not in the buffer pool.
 * \return PAGENOTPINNED if the page's pin count is already zero, indicating an error in the calling code.
 */


const Status BufMgr::unPinPage(File *file, const int PageNo,
                               const bool dirty) {
    int frameNo;
    // 1. Look up the frame in the hash table
    Status status = hashTable->lookup(file, PageNo, frameNo);
    if (status != OK) {
        return HASHNOTFOUND; // The page is not found in the buffer pool, return correspond error code
    }

    // 2. Reference the BufDesc for the found frame
    BufDesc *bufDesc = &bufTable[frameNo];

    // 3. Check the pin count
    if (bufDesc->pinCnt <= 0) {
        // Pin count should not be less than 0; indicate an error if it is
        return PAGENOTPINNED; // The page is not pinned
    }

    // 4. Decrement the pin count and set dirty bit if necessary
    bufDesc->pinCnt--;
    if (dirty) {
        bufDesc->dirty = true; // Mark the page as dirty
    }

    // 5. Return OK on successful operation
    return OK;
}


/**
 * Allocates a new page in a file and a frame in the buffer pool for the page. The function first allocates an empty page
 * in the specified file. It then allocates a buffer frame for this page using the clock  algorithm.
 * After obtaining a frame, it inserts an entry for the new page into the hash table and sets up the frame's
 * metadata. If any step fails (allocating a page in the file, allocating a buffer frame, or inserting into the hash table),
 * it performs necessary cleanup, such as deallocating the page from the file and clearing the frame's metadata.
 *
 * \param[in] file A pointer to the File object where the new page will be created.
 * \param[out] pageNo An integer reference where the number of the newly allocated page will be stored.
 * \param[out] page A pointer to a Page pointer, which will point to the buffer frame containing the newly allocated page upon success.
 * \return OK if the page and frame were successfully allocated and set up.
 * \return BUFFEREXCEEDED if no buffer frame could be allocated because all frames are pinned.
 * \return UNIXERR if there's a file-level error during page allocation.
 * \return HASHTBLERROR if inserting the new page into the hash table fails.
 */

const Status BufMgr::allocPage(File *file, int &pageNo, Page *&page) {
    Status status;
    int frameNo;

    // 1. Allocate an empty page in the file
    status = file->allocatePage(pageNo);
    if (status != OK) {
        return status; // RETURN by allocatePage() in db.C
    }

    // 2. Obtain a buffer pool frame
    status = allocBuf(frameNo);
    if (status != OK) {
        // If no frame could be allocated, it's necessary to deallocate the page in the file
        file->disposePage(pageNo);
        return status; // // Could be BUFFEREXCEEDED or UNIXERR
    }

    // 3. Setup the buffer frame properly
    bufTable[frameNo].Set(file, pageNo);

    // 4. Get a pointer to the allocated frame in the buffer pool
    page = &bufPool[frameNo];

    // 5. Insert an entry into the hash table for quick look up
    status = hashTable->insert(file, pageNo, frameNo);
    if (status != OK) {
        // Cleanup if the hash table insertion fails
        bufTable[frameNo].Clear(); // Clear the buffer descriptor to make it available again
        file->disposePage(pageNo); // Deallocate the page from the file
        return HASHTBLERROR;
    }

    // 6. Successful allocation and setup, returns the page number and frame pointer to the caller
    return OK;
}

const Status BufMgr::disposePage(File *file, const int pageNo) {
    // see if it is in the buffer pool
    Status status = OK;
    int frameNo = 0;
    status = hashTable->lookup(file, pageNo, frameNo);
    if (status == OK) {
        // clear the page
        bufTable[frameNo].Clear();
    }
    status = hashTable->remove(file, pageNo);

    // deallocate it in the file
    return file->disposePage(pageNo);
}

const Status BufMgr::flushFile(const File *file) {
    Status status;

    for (int i = 0; i < numBufs; i++) {
        BufDesc *tmpbuf = &(bufTable[i]);
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

            hashTable->remove(file, tmpbuf->pageNo);

            tmpbuf->file = NULL;
            tmpbuf->pageNo = -1;
            tmpbuf->valid = false;
        } else if (tmpbuf->valid == false && tmpbuf->file == file)
            return BADBUFFER;
    }

    return OK;
}


void BufMgr::printSelf(void) {
    BufDesc *tmpbuf;

    cout << endl << "Print buffer...\n";
    for (int i = 0; i < numBufs; i++) {
        tmpbuf = &(bufTable[i]);
        cout << i << "\t" << (char *) (&bufPool[i])
             << "\tpinCnt: " << tmpbuf->pinCnt;

        if (tmpbuf->valid == true)
            cout << "\tvalid\n";
        cout << endl;
    };
}


