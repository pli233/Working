#include "heapfile.h"
#include "error.h"

//TODO:
//1. createHeapFile()
//2. HeapFile()
//3. getRecord()
//4. scanNext()
//5. insertRecord()


/* This function creates an empty (well, almost empty) heap file.
 * To do this create a db level file by calling db->createfile().
 * Then, allocate an empty page by invoking bufMgr->allocPage() appropriately.
 * As you know allocPage() will return a pointer to an empty page in the buffer pool
 * along with the page number of the page. Take the Page* pointer returned from allocPage()
 * and cast it to a FileHdrPage*. Using this pointer initialize the values in the header page.
 * Then make a second call to bufMgr->allocPage(). This page will be the first data page of the file.
 * Using the Page* pointer returned, invoke its init() method to initialize the page contents.
 * Finally, store the page number of the data page in firstPage and lastPage attributes of the FileHdrPage.
 * When you have done all this unpin both pages and mark them as dirty.
 *
 * It is associate with the FileHdrPage class
 */



// routine to create a heapfile
const Status createHeapFile(const string fileName)
{
    //Predefined variables
    File* 		file;
    Status 		status;
    FileHdrPage*	hdrPage;
    int			hdrPageNo;
    int			newPageNo;
    Page*		newPage;

    // Attempt to open the file to see if it already exists
    status = db.openFile(fileName, file);
    if (status != OK)
    {
        cout << "My debug: File does not exist, attempting to create it." << endl;
        // File doesn't exist, so first create it
        status = db.createFile(fileName);
        if (status != OK) {
            cerr << "My debug: Error creating file: " << fileName << endl;
            return status;
        }

        // Open the newly created file
        status = db.openFile(fileName, file);
        if (status != OK) {
            cerr << "My debug: Error opening new file after creation: " << fileName << endl;
            return status;
        }

        // Allocate an empty header page
        status = bufMgr->allocPage(file, hdrPageNo, (Page*&)hdrPage);
        if (status != OK) {
            cerr << "Error allocating header page for file: " << fileName << endl;
            db.closeFile(file);  // Ensure file is closed on error
            return status;
        }

        // Initialize the header page
        hdrPage->firstPage = -1;
        hdrPage->lastPage = -1;
        hdrPage->pageCnt = 0;
        hdrPage->recCnt = 0;
        strncpy(hdrPage->fileName, fileName.c_str(), MAXNAMESIZE);

        // Allocate a first data page
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) {
            cerr << "My debug: Error allocating first data page for file: " << fileName << endl;
            bufMgr->unPinPage(file, hdrPageNo, false);  // unpin header page if data page allocation fails
            db.closeFile(file);  // Ensure file is closed on error
            return status;
        }

        // Initialize the data page
        newPage->init(newPageNo);

        // Update header page
        hdrPage->firstPage = newPageNo;
        hdrPage->lastPage = newPageNo;
        hdrPage->pageCnt = 2; // including the header page itself
        hdrPage->recCnt = 0;  // no records yet

        // Unpin both pages and mark them as dirty
        bufMgr->unPinPage(file, hdrPageNo, true);
        bufMgr->unPinPage(file, newPageNo, true);

        // Close the file after creation
        db.closeFile(file);
        return OK;
    }
    // If file exists, close it and return error
    db.closeFile(file);
    return (FILEEXISTS);
}


// routine to destroy a heapfile
// This is easy. Simply call db->destroyFile().
// The user is expected to have closed all instances of the file before calling this function.
// It is associate with the FileHdrPage class
const Status destroyHeapFile(const string fileName)
{
	return (db.destroyFile (fileName));
}


/* This method first opens the appropriate file by calling db.openFile()
 * (do not forget to save the File* returned in the filePtr data member).
 * Next, it reads and pins the header page for the file in the buffer pool,
 * initializing the private data members headerPage, headerPageNo, and hdrDirtyFlag.
 * You might be wondering how you get the page number of the header page.
 * This is what file->getFirstPage() is used for (see description of the I/O layer)!
 * Finally, read and pin the first page of the file into the buffer pool, initializing the values of curPage,
 * curPageNo, and curDirtyFlag appropriately. Set curRec to NULLRID.
 *
 */

// constructor opens the underlying file
HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;
    Page*	pagePtr;

    cout << "opening file " << fileName << endl;

    // open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
        // Get the first page of the file, which is the header page
        int headerPageNo;
        if ((status = filePtr->getFirstPage(headerPageNo)) != OK) {
            cerr << "Failed to get header page\n";
            returnStatus = status;
            db.closeFile(filePtr);  // Clean up: close the file if getting first page fails
            return;
        }

        // Read and pin the header page
        if ((status = bufMgr->readPage(filePtr, headerPageNo, (Page*&)headerPage)) != OK) {
            cerr << "Failed to pin header page\n";
            returnStatus = status;
            db.closeFile(filePtr);  // Clean up: close the file if pinning header page fails
            return;
        }

        // Initialize header page details
        this->headerPageNo = headerPageNo;
        this->hdrDirtyFlag = false;  // Initially, the header page is not modified

        // Read and pin the first data page, using the 'firstPage' from the header page
        if (headerPage->firstPage != -1) {
            int firstDataPageNo = headerPage->firstPage;
            if ((status = bufMgr->readPage(filePtr, firstDataPageNo, curPage)) != OK) {
                cerr << "Failed to pin first data page\n";
                bufMgr->unPinPage(filePtr, headerPageNo, false); // Unpin header page on failure
                db.closeFile(filePtr);  // Clean up: close the file if pinning data page fails
                returnStatus = status;
                return;
            }
            // Initialize current page details
            curPageNo = firstDataPageNo;
            curDirtyFlag = false; // Initially, the data page is not modified
            curRec = NULLRID;     // No record selected initially
        } else {
            curPage = nullptr;
            curPageNo = -1;
            curDirtyFlag = false;
            curRec = NULLRID;
        }
        cout << "Mydebug: HeapFile function success, return OK\n" << fileName << endl;
        returnStatus = OK;  // Successful initialization
    }
    else
    {
        cerr << "open of heap file failed\n";
        returnStatus = status;
        return;
    }
}

// the destructor closes the file
HeapFile::~HeapFile()
{
    Status status;
    cout << "invoking heapfile destructor on file " << headerPage->fileName << endl;

    // see if there is a pinned data page. If so, unpin it 
    if (curPage != NULL)
    {
    	status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
		curPage = NULL;
		curPageNo = 0;
		curDirtyFlag = false;
		if (status != OK) cerr << "error in unpin of date page\n";
    }
	
	 // unpin the header page
    status = bufMgr->unPinPage(filePtr, headerPageNo, hdrDirtyFlag);
    if (status != OK) cerr << "error in unpin of header page\n";
	
	// status = bufMgr->flushFile(filePtr);  // make sure all pages of the file are flushed to disk
	// if (status != OK) cerr << "error in flushFile call\n";
	// before close the file
	status = db.closeFile(filePtr);
    if (status != OK)
    {
		cerr << "error in closefile call\n";
		Error e;
		e.print (status);
    }
}

// Return number of records in heap file

const int HeapFile::getRecCnt() const
{
  return headerPage->recCnt;
}

// retrieve an arbitrary record from a file.
// if record is not on the currently pinned page, the current page
// is unpinned and the required page is read into the buffer pool
// and pinned.  returns a pointer to the record via the rec parameter

/* This method returns a record (via the rec structure) given the RID of the record.
 * The private data members curPage and curPageNo should be used to keep track of the current
 * data page pinned in the buffer pool. If the desired record is on the currently pinned page, simply invoke
 * curPage->getRecord(rid, rec) to get the record.
 *
 * Otherwise, you need to unpin the currently pinned page (assuming a page is pinned) and use the
 * pageNo field of the RID to read the page into the buffer pool.
 */
const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;

    // cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;


    // Check if the current page is already the page containing the record
    if (curPageNo == rid.pageNo)
    {
        // If the record is on the current page, fetch the record directly
        status = curPage->getRecord(rid, rec);
        if (status != OK) {
            cerr << "Failed to get record from current page\n";
            return status;
        }
    }
    else
    {
        // If the record is not on the current page, unpin the current page if it's pinned
        if (curPage != nullptr && curPageNo != -1)
        {
            status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if (status != OK) {
                cerr << "Failed to unpin current page\n";
                return status;
            }
            curPage = nullptr; // Make sure to reset the curPage pointer after unpinning
            curPageNo = -1;
            curDirtyFlag = false;
        }

        // Pin the new page containing the record
        status = bufMgr->readPage(filePtr, rid.pageNo, curPage);
        if (status != OK) {
            cerr << "Failed to read and pin new page\n";
            return status;
        }
        curPageNo = rid.pageNo;
        curDirtyFlag = false;  // Reset dirty flag since we just loaded the page

        // Now fetch the record from the new page
        status = curPage->getRecord(rid, rec);
        if (status != OK) {
            cerr << "Failed to get record from new page\n";
            bufMgr->unPinPage(filePtr, rid.pageNo, false);  // Unpin the new page if record fetch failed
            return status;
        }
    }

    return OK;
   
   
   
   
}

HeapFileScan::HeapFileScan(const string & name,
			   Status & status) : HeapFile(name, status)
{
    filter = NULL;
}

const Status HeapFileScan::startScan(const int offset_,
				     const int length_,
				     const Datatype type_, 
				     const char* filter_,
				     const Operator op_)
{
    if (!filter_) {                        // no filtering requested
        filter = NULL;
        return OK;
    }
    
    if ((offset_ < 0 || length_ < 1) ||
        (type_ != STRING && type_ != INTEGER && type_ != FLOAT) ||
        (type_ == INTEGER && length_ != sizeof(int)
         || type_ == FLOAT && length_ != sizeof(float)) ||
        (op_ != LT && op_ != LTE && op_ != EQ && op_ != GTE && op_ != GT && op_ != NE))
    {
        return BADSCANPARM;
    }

    offset = offset_;
    length = length_;
    type = type_;
    filter = filter_;
    op = op_;

    return OK;
}


const Status HeapFileScan::endScan()
{
    Status status;
    // generally must unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        curPage = NULL;
        curPageNo = 0;
		curDirtyFlag = false;
        return status;
    }
    return OK;
}

HeapFileScan::~HeapFileScan()
{
    endScan();
}

const Status HeapFileScan::markScan()
{
    // make a snapshot of the state of the scan
    markedPageNo = curPageNo;
    markedRec = curRec;
    return OK;
}

const Status HeapFileScan::resetScan()
{
    Status status;
    if (markedPageNo != curPageNo) 
    {
		if (curPage != NULL)
		{
			status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
			if (status != OK) return status;
		}
		// restore curPageNo and curRec values
		curPageNo = markedPageNo;
		curRec = markedRec;
		// then read the page
		status = bufMgr->readPage(filePtr, curPageNo, curPage);
		if (status != OK) return status;
		curDirtyFlag = false; // it will be clean
    }
    else curRec = markedRec;
    return OK;
}

/* Returns (via the outRid parameter) the RID of the next record that satisfies the scan predicate.
 * The basic idea is to scan the file one page at a time. For each page, use the firstRecord()
 * and nextRecord() methods of the Page class to get the rids of all the records on the page.
 * Convert the rid to a pointer to the record data and invoke matchRec() to determine if record satisfies
 * the filter associated with the scan. If so, store the rid in curRec and return curRec.
 * To make things fast, keep the current page pinned until all the records on the page have been processed.
 * Then continue with the next page in the file.  Since the HeapFileScan class is derived from the HeapFile class
 * it also has all the methods of the HeapFile class as well. Returns OK if no errors occurred. Otherwise,
 * return the error code of the first error that occurred.
 */
const Status HeapFileScan::scanNext(RID& outRid)
{
    Status status = OK;
    Record rec;
    bool found = false;

    // Continue scanning from the current record, or start at the first record of the current page
    while (!found) {
        // If the current page is not loaded or there's no current record, start with the first record
        if (curPage == nullptr || curRec.pageNo == -1) {
            if (curPage == nullptr) { // If no page is pinned
                if (curPageNo == -1) {
                    // Start scanning from the first page
                    curPageNo = headerPage->firstPage;
                    if (curPageNo == -1) return FILEEOF; // No pages to scan
                }
                status = bufMgr->readPage(filePtr, curPageNo, curPage);
                if (status != OK) return status;
            }
            status = curPage->firstRecord(curRec);
            if (status == NORECORDS) { // No records on this page
                // Move to the next page
                int nextPageNo;
                status = curPage->getNextPage(nextPageNo);
                bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
                if (status != OK || nextPageNo == -1) return FILEEOF;
                curPage = nullptr;
                curPageNo = nextPageNo;
                continue;
            } else if (status != OK) {
                return status;
            }
        } else {
            // Move to the next record in the current page
            status = curPage->nextRecord(curRec, curRec);
            if (status == ENDOFPAGE) {
                // Move to the next page
                int nextPageNo;
                status = curPage->getNextPage(nextPageNo);
                bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
                if (status != OK || nextPageNo == -1) return FILEEOF;
                curPage = nullptr;
                curPageNo = nextPageNo;
                continue;
            } else if (status != OK) {
                return status;
            }
        }

        // Retrieve the record
        status = curPage->getRecord(curRec, rec);
        if (status != OK) return status;

        // Check if the record matches the filter criteria
        if (matchRec(rec)) {
            outRid = curRec;
            return OK;
        }
    }

    return FILEEOF; // If no records match after scanning all pages
}




// returns pointer to the current record.  page is left pinned
// and the scan logic is required to unpin the page 

const Status HeapFileScan::getRecord(Record & rec)
{
    return curPage->getRecord(curRec, rec);
}

// delete record from file. 
const Status HeapFileScan::deleteRecord()
{
    Status status;

    // delete the "current" record from the page
    status = curPage->deleteRecord(curRec);
    curDirtyFlag = true;

    // reduce count of number of records in the file
    headerPage->recCnt--;
    hdrDirtyFlag = true; 
    return status;
}


// mark current page of scan dirty
const Status HeapFileScan::markDirty()
{
    curDirtyFlag = true;
    return OK;
}

const bool HeapFileScan::matchRec(const Record & rec) const
{
    // no filtering requested
    if (!filter) return true;

    // see if offset + length is beyond end of record
    // maybe this should be an error???
    if ((offset + length -1 ) >= rec.length)
	return false;

    float diff = 0;                       // < 0 if attr < fltr
    switch(type) {

    case INTEGER:
        int iattr, ifltr;                 // word-alignment problem possible
        memcpy(&iattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ifltr,
               filter,
               length);
        diff = iattr - ifltr;
        break;

    case FLOAT:
        float fattr, ffltr;               // word-alignment problem possible
        memcpy(&fattr,
               (char *)rec.data + offset,
               length);
        memcpy(&ffltr,
               filter,
               length);
        diff = fattr - ffltr;
        break;

    case STRING:
        diff = strncmp((char *)rec.data + offset,
                       filter,
                       length);
        break;
    }

    switch(op) {
    case LT:  if (diff < 0.0) return true; break;
    case LTE: if (diff <= 0.0) return true; break;
    case EQ:  if (diff == 0.0) return true; break;
    case GTE: if (diff >= 0.0) return true; break;
    case GT:  if (diff > 0.0) return true; break;
    case NE:  if (diff != 0.0) return true; break;
    }

    return false;
}

InsertFileScan::InsertFileScan(const string & name,
                               Status & status) : HeapFile(name, status)
{
  //Do nothing. Heapfile constructor will bread the header page and the first
  // data page of the file into the buffer pool
}

InsertFileScan::~InsertFileScan()
{
    Status status;
    // unpin last page of the scan
    if (curPage != NULL)
    {
        status = bufMgr->unPinPage(filePtr, curPageNo, true);
        curPage = NULL;
        curPageNo = 0;
        if (status != OK) cerr << "error in unpin of data page\n";
    }
}

// Insert a record into the file
/* TIPS: check if curPage is NULL. If so, make the last page the current page and read it
 * into the buffer. Call curPage->insertRecord to insert the record.
 *
 * If successful, remember to DO THE BOOKKEEPING.
 * That is, you have to update data fields such as recCnt, hdrDirtyFlag, curDirtyFlag, etc.
 * If can't insert into the current page, then create a new page, initialize it properly,
 * modify the header page content properly, link up the new page appropriately, make the current
 * page to be the newly allocated page, then try to insert the record. Don't forget bookkeeping
 * that must be done after successfully inserting the record.
 */
const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    RID		rid;

    // check for very large records
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    // If curPage is NULL, try to use the last page of the file
    if (curPage == nullptr) {
        if (headerPage->lastPage != -1) {
            status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
            if (status != OK) return status;
            curPageNo = headerPage->lastPage;
            curDirtyFlag = false;  // New page, no modifications yet
        } else {
            // If there are no pages in the file, create a new first page
            status = bufMgr->allocPage(filePtr, newPageNo, newPage);
            if (status != OK) return status;
            newPage->init(newPageNo);
            headerPage->firstPage = newPageNo;
            headerPage->lastPage = newPageNo;
            curPage = newPage;
            curPageNo = newPageNo;
            curDirtyFlag = true; // New page is dirty
            hdrDirtyFlag = true; // Header page updated
        }
    }

    // Try to insert the record into the current page
    status = curPage->insertRecord(rec, outRid);
    if (status == OK) {
        headerPage->recCnt++;       // Update record count in the file header
        hdrDirtyFlag = true;        // Header page has been modified
        curDirtyFlag = true;        // Current page has been modified
        return OK;
    } else if (status == NOSPACE) {
        // Allocate a new page if no space on the current page
        status = bufMgr->allocPage(filePtr, newPageNo, newPage);
        if (status != OK) return status;
        newPage->init(newPageNo);

        // Link the new page correctly
        curPage->setNextPage(newPageNo);

        // Update the header page to reflect this new page as the last page
        headerPage->lastPage = newPageNo;
        hdrDirtyFlag = true;

        // Unpin the current page and switch to the new page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if (status != OK) return status;

        // Pin the new page as the current page
        curPage = newPage;
        curPageNo = newPageNo;
        curDirtyFlag = true; // New page is dirty

        // Try to insert the record again
        status = curPage->insertRecord(rec, outRid);
        if (status != OK) return status;

        headerPage->recCnt++;       // Update record count in the file header
        return OK;
    } else {
        // Some other error occurred
        return status;
    }
}


