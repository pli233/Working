#include "heapfile.h"
#include "error.h"

//TODO:
//1. createHeapFile()   passed test and comments
//2. HeapFile()         passed test and comments
//3. insertRecord()     passed test and comments
//4. getRecord()        passed test and comments
//5. scanNext()



/**
 * Creates a new heap file, which includes initializing its header and first data page.
 * This function first checks if the file already exists by attempting to open it. If the file does not exist,
 * it will be created and opened. The function then allocates and initializes a header page and a first data page
 * in the buffer managed by bufMgr. These pages are then configured appropriately for a new heap file.
 * The header page includes details like the page count and record count, and it tracks the pages in the heap file.
 * Both pages are marked as dirty and unpinned after their usage to ensure they are written back to the disk.
 *
 * The function associates with the FileHdrPage class and operates through several database and buffer management
 * interactions to establish the file structure.
 *
 * \return OK on successful creation of the heap file.
 * \return FILEEXISTS if the file already exists and cannot be overwritten.
 * \return Error codes from lower-level functions like db.createFile, bufMgr->allocPage if problems occur during file creation or page allocation.
 * \param[out] fileName The name of the file to create or check for existence.
 */

const Status createHeapFile(const string fileName)
{
    //Predefined variables
    File* 		file;          //the file descriptor opened by db
    Status 		status;        //status code for function to return
    FileHdrPage*	hdrPage;   //pointer to header page
    int			hdrPageNo;     //page number of header page
    int			newPageNo;     //page number of first allocated data page
    Page*		newPage;       //pointer to first allocated data page

    //1. Checks if the file already exists by attempting to open it
    status = db.openFile(fileName, file);
    if (status != OK)
    {
        //cout << "My debug: File does not exist, attempting to create it." << endl;

        //2. If the file does not exist, it will be created and opened.
        status = db.createFile(fileName);
        if (status != OK) {
            //cerr << "My debug: Error creating file: " << fileName << endl;
            return status;
        }

        // Open the newly created file
        status = db.openFile(fileName, file);
        if (status != OK) {
            //cerr << "My debug: Error opening new file after creation: " << fileName << endl;
            return status;
        }

        //3. Allocates and initializes a header page and a first data page in the buffer managed by bufMgr.

        //3.1 Allocate a header page
        status = bufMgr->allocPage(file, hdrPageNo, (Page*&)hdrPage);
        if (status != OK) {
            //cerr << "Mydebug: Error allocating header page for file: " << fileName << endl;
            db.closeFile(file);  // Ensure file is closed on error
            return status;
        }

        // Initialize the header page properly
        hdrPage->firstPage = -1;
        hdrPage->lastPage = -1;
        hdrPage->pageCnt = 0;
        hdrPage->recCnt = 0;
        strncpy(hdrPage->fileName, fileName.c_str(), MAXNAMESIZE);



        //3.2 Allocate a first data page
        status = bufMgr->allocPage(file, newPageNo, newPage);
        if (status != OK) {
            //cerr << "My debug: Error allocating first data page for file: " << fileName << endl;
            bufMgr->unPinPage(file, hdrPageNo, false);  // unpin header page if data page allocation fails
            db.closeFile(file);  // Ensure file is closed on error
            return status;
        }

        // Initialize the first data page properly
        newPage->init(newPageNo);

        // Update header page
        hdrPage->firstPage = newPageNo;
        hdrPage->lastPage = newPageNo;
        hdrPage->pageCnt = 2; // including the header page itself
        hdrPage->recCnt = 0;  // no records yet


        // 3.3 Finish allocation of header and first data page successfully

        //cerr << "My debug: allocate header and first data page successfully " << endl;
        // Unpin both pages and mark them as dirty
        bufMgr->unPinPage(file, hdrPageNo, true);
        bufMgr->unPinPage(file, newPageNo, true);

        // Close the file after creation
        db.closeFile(file);
        return OK;
    }

    //4. If file exists, close it and return error
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


/**
 * Constructor for HeapFile class that opens an existing heap file or creates a new one if it does not exist.
 * It initializes the class's data members by pinning the header page and the first data page of the heap file into the buffer pool.
 *
 * The method performs the following steps:
 * - Opens the file using db.openFile and saves the returned File pointer in the filePtr member.
 * - Retrieves the header page (the first page of the file) using filePtr->getFirstPage.
 * - Pins the header page in the buffer pool, which involves reading the page into memory and marking it as in-use.
 * - Initializes the headerPage, headerPageNo, and hdrDirtyFlag members from the header page's data.
 * - Attempts to read and pin the first data page of the file. If the header indicates there is a first data page,
 *   it reads and pins this page into memory, setting curPage, curPageNo, curDirtyFlag, and curRec members.
 *
 * On any failure, the method performs necessary cleanup by unpinning pages and closing the file,
 * and it sets the returnStatus output parameter accordingly.
 *
 * @param fileName The name of the file to open as a heap file.
 * @param[out] returnStatus The status of the operation: OK, FILENOTFOUND, or any other errors encountered.
 */


HeapFile::HeapFile(const string & fileName, Status& returnStatus)
{
    Status 	status;     // Operation status code
    Page*	pagePtr;    // Temporary pointer to the page read from the file

    cout << "opening file " << fileName << endl;

    //1. open the file and read in the header page and the first data page
    if ((status = db.openFile(fileName, filePtr)) == OK)
    {
        // file read success, now filePtr has information we need

        //2. Get the first page of the file, which is the header page
        int headerPageNo;

        //2.1 Retrieves the header page (the first page of the file) using filePtr->getFirstPage.
        if ((status = filePtr->getFirstPage(headerPageNo)) != OK) {
            //cerr << "Mydebug: Failed to get header page\n";
            returnStatus = status;
            db.closeFile(filePtr);  // Clean up: close the file if getting first page fails
            return;
        }

        //2.2 Read and pin the header page
        if ((status = bufMgr->readPage(filePtr, headerPageNo, (Page*&)headerPage)) != OK) {
            //cerr << "MY debug: Failed to pin header page\n";
            returnStatus = status;
            db.closeFile(filePtr);  // Clean up: close the file if pinning header page fails
            return;
        }

        // Get, read, and pinned header page successfully

        // Initialize header page details to the heapfile object, headerPage was set when read in 2.2
        this->headerPageNo = headerPageNo;
        this->hdrDirtyFlag = false;  // Initially, the header page is not modified


        // 3. Read and pin the first page of the file into the buffer pool,
        // initializing the values of curPage, curPageNo, and curDirtyFlag appropriately. Set curRec to NULLRID.

        // 3.1 Read and pin the first data page, using the 'firstPage' from the header page
        if (headerPage->firstPage != -1) {
            int firstDataPageNo = headerPage->firstPage;

            //Check read works
            if ((status = bufMgr->readPage(filePtr, firstDataPageNo, curPage)) != OK) {
                //cerr << "Mydebug: Failed to pin first data page\n";
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
        //cout << "Mydebug: HeapFile function success, return OK\n" << fileName << endl;
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

/**
 * Retrieves a specified record from the heap file based on its Record ID (RID).
 * This method manages the buffer pool by ensuring that the correct page containing the record is pinned.
 * If the desired record is located on the current page (curPage), the record is fetched directly.
 * If the record is on a different page or if no page is currently pinned (curPage is nullptr),
 * the method will unpin the current page (if necessary), pin the required page, and then retrieve the record.
 *
 * Tips: this function need to ensure that curpage is which page the record located, so we handle two
 * different cases that will cause error then read
 *
 * Steps:
 * 1. Check if the current page is nullptr (no page is pinned):
 *    - Pin the page containing the requested record directly since no page is currently pinned.
 * 2. If the record's page is different from the current page:
 *    - Unpin the currently pinned page.
 *    - Pin the required page that contains the record.
 * 3. Retrieve the record from the now-current page.
 * 4. Proper bookkeeping is maintained throughout the process, updating curPage, curPageNo, curDirtyFlag,
 *    and curRec to reflect the current state of the buffer pool and the record access.
 *
 * Note:
 * - It is critical to handle the buffer pool carefully to avoid leaving pages unnecessarily pinned
 *   and to manage the dirty flags accurately if modifications have been made.
 * - This function ensures accurate bookkeeping, especially when changing pages, to maintain the integrity of
 *   heap file operations and prevent errors in subsequent accesses.
 *
 * @param rid The Record ID specifying the exact record to retrieve.
 * @param[out] rec A reference to a Record structure where the retrieved record's data will be stored.
 * @return Status OK if the record retrieval is successful, FAILED if the record cannot be retrieved,
 *         or other error codes as appropriate for different failure scenarios.
 */


const Status HeapFile::getRecord(const RID & rid, Record & rec)
{
    Status status;

    // cout<< "getRecord. record (" << rid.pageNo << "." << rid.slotNo << ")" << endl;


    //1. Check if the current page is nullptr (no page is pinned):
    if (curPage == nullptr)
    {

        // Directly read and pin the page that contains the record since no page is currently pinned
        status = bufMgr->readPage(filePtr, rid.pageNo, curPage);
        if (status != OK) {
            cerr << "Failed to read and pin page\n";
            return status;
        }

        curPageNo = rid.pageNo; // Update curPageNo to the new page
        curDirtyFlag = false;  // Reset dirty flag since we just loaded the page
    }
    //2. If the record's page is different from the current page:
    else if (curPageNo != rid.pageNo)
    {
        //2.1 The record is not on the current page, unpin the current page
        status = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if (status != OK) {
            //cerr << "My debug: Failed to unpin current page\n";
            return status;
        }

        //2.2 Reset current page tracking since we're moving to another page
        curPage = nullptr;
        curPageNo = -1;
        curDirtyFlag = false;

        //2.3 Pin the new page containing the record
        status = bufMgr->readPage(filePtr, rid.pageNo, curPage);
        if (status != OK) {
            //cerr << "Mydebug: Failed to read and pin new page\n";
            return status;
        }

        curPageNo = rid.pageNo; // Update curPageNo to the new page
        curDirtyFlag = false;  // Reset dirty flag since we just loaded the page
    }
    else{} // curpage is the exact one with the rid we want, don't do anything

    // 3. Retrieve the record from the now-current page, now curpage is the exact one with the rid we want
    status = curPage->getRecord(rid, rec);
    if (status != OK) {
        cerr << "Failed to get record from page\n";
        if (curPage != nullptr) {
            bufMgr->unPinPage(filePtr, curPageNo, false);  // Unpin the page if record fetch failed
        }
        return status;
    }

    //4. Finish get record and return
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

/**
 * Scans the heap file to find and return the Record ID (RID) of the next record that matches a predefined scan predicate.
 * Iterates over pages and records sequentially, using methods to traverse and check records against a predicate.
 * Pages are kept pinned while scanning, and are unpinned as needed when moving to the next page.
 *
 * @param outRid Reference to an RID structure to store the RID of the matching record.
 * @return Status OK if a matching record is found, FILEEOF if no matching records are found, or an error code.
 */

const Status HeapFileScan::scanNext(RID& outRid)
{
    Status  status = OK;
    RID     nextRid;
    RID     tmpRid;
    int     nextPageNo;
    Record  rec;

    while (true) {

        //1. Load the current page if it's not already loaded
        if (curPage == nullptr) {
            //1.1 Check if we need to start from the first page
            if (curPageNo == -1) {
                curPageNo = headerPage->firstPage;
                // Return end of file if there are no pages to scan
                if (curPageNo == -1) return FILEEOF;
            }
            //1.2 Read the current page content
            status = bufMgr->readPage(filePtr, curPageNo, curPage);
            if (status != OK) return status;
        }

        //2.  Choose to fetch either the first or the next record on the current page
        if (curRec.pageNo == -1) {
            status = curPage->firstRecord(curRec);
        } else {
            status = curPage->nextRecord(curRec, curRec);
        }

        //3. Handle the end of records or the page
        if (status == NORECORDS || status == ENDOFPAGE) {
            // Attempt to move to the next page
            status = curPage->getNextPage(nextPageNo);
            // Unpin the current page before moving to the next
            bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
            if (status != OK || nextPageNo == -1) return FILEEOF;  // Return if there are no more pages or an error occurred
            // Reset page pointers and continue with the new page
            curPage = nullptr;
            curPageNo = nextPageNo;
            curRec.pageNo = -1;
            continue;
        } else if (status != OK) {
            return status;
        }

        //4. Retrieve the current record
        status = curPage->getRecord(curRec, rec);
        if (status != OK) return status;

        //5. Check if the record matches the scan criteria
        if (matchRec(rec)) {
            outRid = curRec;  // Return the RID of the matching record
            return OK;
        }
    }
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

/**
 * Inserts a record into the heap file. This method handles the insertion by checking the current page
 * (curPage) for available space and inserting the record there if possible. If curPage is null or full,
 * it either finds the last page of the file to insert the record or allocates a new page if necessary.
 *
 * Steps:
 * 1. Check if the record size is too large for a page.
 * 2. If curPage is NULL, load the last page of the file into the buffer.
 * 3. If there is no last page (empty file), allocate a new page and initialize it.
 * 4. Attempt to insert the record into the current page.
 * 5. If the current page has no space, allocate a new page and link it properly.
 * 6. Update headerPage and other bookkeeping information after a successful insert.
 *
 * Note:
 * - The method ensures that if a new page is created, it is properly initialized and linked
 *   to the file's page structure.
 * - Bookkeeping includes updating the record count, setting the dirty flags for pages, and
 *   updating the header page with new information about the first and last pages of the file.
 * - The method accounts for errors by returning appropriate Status codes.
 *
 * @param rec The record to insert into the file.
 * @param[out] outRid The record ID where the record is inserted.
 * @return Status OK if the insert is successful, NOSPACE if there is no space on the current page,
 *         INVALIDRECLEN if the record is too large, or other error codes based on the failure encountered.
 */

const Status InsertFileScan::insertRecord(const Record & rec, RID& outRid)
{
    Page*	newPage;
    int		newPageNo;
    Status	status, unpinstatus;
    RID		rid;

    // 1. Check if the record size is too large for a page.
    if ((unsigned int) rec.length > PAGESIZE-DPFIXED)
    {
        // will never fit on a page, so don't even bother looking
        return INVALIDRECLEN;
    }

    // 2. If curPage is NULL, load the last page of the file into the buffer.
    if (curPage == nullptr) {

        // 2.1 Try to load the lastPage
        if (headerPage->lastPage != -1) {
            status = bufMgr->readPage(filePtr, headerPage->lastPage, curPage);
            if (status != OK) return status;
            curPageNo = headerPage->lastPage;
            curDirtyFlag = false;  // New page, no modifications yet
        } else {
            //3. If there is no last page (empty file), allocate a new page and initialize it.
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

    //4. Curpage is not empty, Try to insert the record into the current page
    status = curPage->insertRecord(rec, rid); //using rid as temp for safety
    if (status == OK) {
        outRid = rid;               // Use the temporary rid to update outRid for the caller
        headerPage->recCnt++;       // Update record count in the file header
        hdrDirtyFlag = true;        // Header page has been modified
        curDirtyFlag = true;        // Current page has been modified
        return OK;
    }
    //5. If the current page has no space, allocate a new page and link it properly.
    else if (status == NOSPACE) {

        // Allocate a new page if no space on the current page
        status = bufMgr->allocPage(filePtr, newPageNo, newPage);
        if (status != OK) return status;


        //6. Update headerPage and other bookkeeping information after a successful insert.
        newPage->init(newPageNo);

        // Link the new page correctly
        curPage->setNextPage(newPageNo);

        // Update the header page to reflect this new page as the last page
        headerPage->lastPage = newPageNo;
        hdrDirtyFlag = true;

        // Unpin the current page and switch to the new page
        unpinstatus = bufMgr->unPinPage(filePtr, curPageNo, curDirtyFlag);
        if (unpinstatus != OK) return unpinstatus;

        // Pin the new page as the current page
        curPage = newPage;
        curPageNo = newPageNo;
        curDirtyFlag = true; // New page is dirty

        // Try to insert the record again
        status = curPage->insertRecord(rec, rid);
        if (status != OK) return status;

        // Use the temporary rid to update outRid for the caller
        outRid = rid;
        headerPage->recCnt++;       // Update record count in the file header
        return OK;
    } else {
        // Some other error occurred
        return status;
    }
}



