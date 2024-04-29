#include "catalog.h"
#include "query.h"


// forward declaration
const Status ScanSelect(const string & result, 
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen);

/*
 * Selects records from the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
    // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;


    // Array to hold attribute descriptions for projection
    AttrDesc *projDesc = new AttrDesc[projCnt];
    Status status;

    // Get attribute descriptions for projection list
    for (int i = 0; i < projCnt; i++) {
        status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, projDesc[i]);
        if (status != OK) {
            cerr << "Error retrieving attribute info for " << projNames[i].attrName << endl;
            delete[] projDesc;
            return status;
        }
    }

    // If there is a selection attribute specified, get its description
    AttrDesc attrDesc;
    if (attr != NULL) {
        status = attrCat->getInfo(attr->relName, attr->attrName, attrDesc);
        if (status != OK) {
            cerr << "Error retrieving attribute info for selection attribute " << attr->attrName << endl;
            delete[] projDesc;
            return status;
        }
    }

    // Calculate record length for the result projection
    int reclen = 0;
    for (int i = 0; i < projCnt; i++) {
        reclen += projDesc[i].attrLen;
    }

    // Call ScanSelect to perform the actual selection and projection
    if (attr != NULL) {
        status = ScanSelect(result, projCnt, projDesc, &attrDesc, op, attrValue, reclen);
    } else {
        // Perform an unconditional scan if no selection attribute is provided
        status = ScanSelect(result, projCnt, projDesc, NULL, op, NULL, reclen);
    }

    // Cleanup and return status
    delete[] projDesc;
    if (status != OK) {
        cerr << "ScanSelect failed with status " << status << endl;
    } else {
        cout << "QU_Select completed successfully." << endl;
    }
    return status;
}


const Status ScanSelect(const string & result, 
#include "stdio.h"
#include "stdlib.h"
			const int projCnt, 
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;


    Status status;
    RID rid;
    Record record;

    // Setup output table using InsertFileScan
    InsertFileScan ifs(result, status);
    if (status != OK) {
        cerr << "Error opening InsertFileScan on result table." << endl;
        return status;
    }

    // Configure the HeapFileScan
    HeapFileScan hfs(attrDesc ? attrDesc->relName : "", status);
    if (status != OK) {
        cerr << "Error opening HeapFileScan on source table." << endl;
        return status;
    }

    // Start scan (either filtered or unfiltered)
    if (attrDesc != NULL && filter != NULL) {
        status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen,
                               static_cast<Datatype>(attrDesc->attrType), filter, op);
        if (status != OK) {
            cerr << "Error starting filtered scan on source table." << endl;
            return status;
        }
    } else {
        // Start an unfiltered scan assuming the first attribute is always valid
        status = hfs.startScan(0, 0, static_cast<Datatype>(attrDesc ? attrDesc->attrType : INTEGER), NULL, op);
        if (status != OK) {
            cerr << "Error starting unfiltered scan on source table." << endl;
            return status;
        }
    }

    // Scan through the records
    while ((status = hfs.scanNext(rid)) == OK) {
        status = hfs.getRecord(record);
        if (status != OK) {
            cerr << "Error getting record during scan." << endl;
            break;
        }

        // Prepare a record for insertion
        char *newRecData = new char[reclen];
        Record newRec;
        newRec.data = newRecData;
        newRec.length = reclen;

        int offset = 0;
        for (int i = 0; i < projCnt; i++) {
            AttrDesc projAttr = projNames[i];
            memcpy(newRecData + offset, (char*)record.data + projAttr.attrOffset, projAttr.attrLen);
            offset += projAttr.attrLen;
        }

        // Insert the new record
        RID newRid;
        status = ifs.insertRecord(newRec, newRid);
        if (status != OK) {
            cerr << "Error inserting record into result table." << endl;
            delete[] newRecData;
            break;
        }

        delete[] newRecData;
    }

    // Handle scan completion or errors
    if (status != FILEEOF) {
        cerr << "Error during scanning, not completed properly." << endl;
        return status;
    }

    cout << "ScanSelect completed successfully." << endl;
    return OK;
}
