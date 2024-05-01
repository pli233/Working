#include "catalog.h"
#include "query.h"


/*
 * Inserts a record into the specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Insert(const string & relation, 
	const int attrCnt, 
	const attrInfo attrList[])
{
// Variable declarations
    Status status;             // Will store the return status of functions
    Record outputRec;          // Record object to hold the data to be inserted
    RID rid;                   // Record ID object to be populated by insert function
    int relAttrCnt;            // Count of attributes in the target relation
    AttrDesc *attrDesc;        // Array of attribute descriptions for the relation
    int record_length = 0;     // Total length of the record to be constructed

    // Additional variables for type conversion
    int intVal;                // Store integer values converted from strings
    float floatVal;            // Store float values converted from strings






    //1. Retrieve the attribute descriptions for the target relation
    status = attrCat->getRelInfo(relation, relAttrCnt, attrDesc);
    if (status != OK) {
        return status;         // Return an error if unable to get relation info
    }

    //2. Check if the number of attributes provided matches the relation's attribute count
    if (relAttrCnt != attrCnt) {
        // Incorrect number of attributes provided
        return OK; // It would make more sense to return an error code here instead of OK
    }

    //3. Calculate the total length of the record from attribute lengths
    for (int i = 0; i < attrCnt; i++) {
        record_length += attrDesc[i].attrLen;
    }

    //4. Initialize an insertion file scan for the target relation
    InsertFileScan resultIfs(relation, status);
    if (status != OK) {
        return status;         // Return an error if unable to start the file scan
    }

    //5. Allocate memory for the record data
    outputRec.length = record_length;
    outputRec.data = (char *)malloc(record_length);

    //6. Construct the record by copying data from input attributes to the correct offsets
    for (int i = 0; i < relAttrCnt; i++) {
        for (int j = 0; j < attrCnt; j++) {
            if (strcmp(attrList[j].attrName, attrDesc[i].attrName) == 0) {
                switch (attrList[j].attrType) {
                    case INTEGER:
                        intVal = atoi((char *)attrList[j].attrValue);
                        memcpy((char *)outputRec.data + attrDesc[i].attrOffset, &intVal, attrDesc[i].attrLen);
                        break;
                    case FLOAT:
                        floatVal = atof((char *)attrList[j].attrValue);
                        memcpy((char *)outputRec.data + attrDesc[i].attrOffset, &floatVal, attrDesc[i].attrLen);
                        break;
                    default: // Handle STRING and other types
                        memcpy((char *)outputRec.data + attrDesc[i].attrOffset, attrList[j].attrValue, attrDesc[i].attrLen);
                        break;
                }
                break; // Break the inner loop once the matching attribute is found and processed
            }
        }
    }

    //7. Insert the constructed record into the relation
    resultIfs.insertRecord(outputRec, rid);


    //8. Finish and Clean Up
	
	//Free the allocated memory for the record data
    free(outputRec.data);
    // Return OK to indicate success
    return OK;
}

