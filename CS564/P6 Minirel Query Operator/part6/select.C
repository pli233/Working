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


const Status QU_Select(const string & result, 
		       const int projCnt, 
		       const attrInfo projNames[],
		       const attrInfo *attr, 
		       const Operator op, 
		       const char *attrValue)
{
    // Qu_Select sets up things and then calls ScanSelect to do the actual work
    cout << "Doing QU_Select " << endl;



    //1. Check if the input has a valid attribute type, we only have 0,1,2
    if ((attr != NULL) && (attr->attrType < 0 || attr->attrType > 2)){
		return BADCATPARM;
	}


    //2. Consult the catalog to go from attrInfo to attrDesc (attrCat and relCat, global variables)	    
	Status status;
    AttrDesc *attrDesc;
    int attribute_Count;
    if ((status = attrCat->getRelInfo(projNames[0].relName, attribute_Count, attrDesc)) != OK){
		return status;
	}



	// 3. GO through the projection list and look up each in the attr catalog to get an AttrDesc structure
	int record_length = 0;
	AttrDesc projoect_AttrDesc[projCnt];
	for (int projIndex = 0; projIndex < projCnt; projIndex++){
		for (int attrIndex = 0; attrIndex < attribute_Count; attrIndex++){
			// Check if the attribute is in the projection list
			if (strcmp(attrDesc[attrIndex].attrName, projNames[projIndex].attrName) == 0)
			{
				// Attribute found in the projection list
				// 1) Update record length
				record_length += attrDesc[attrIndex].attrLen;
				// 2) Populate projection attribute description
				projoect_AttrDesc[projIndex] = attrDesc[attrIndex];
				break; // Move to the next attribute in the projection list
			}
		}
	}

	// 4. Find the index of search attribute if specified
	int specified_attr_index  = 0;
	if (attr != NULL){
		for (int attrIndex = 0; attrIndex < attribute_Count; attrIndex++){
			if (strcmp(attrDesc[attrIndex].attrName, attr->attrName) == 0){
				// Search attribute found
				specified_attr_index = attrIndex;
				break; // No need to continue searching, end loop
			}
		}
	}

    // 5. Call up ScanSelect to execute work
    if (attr == NULL)
        return ScanSelect(result, projCnt, projoect_AttrDesc, &projoect_AttrDesc[0], EQ, NULL, record_length);
    else
        return ScanSelect(result, projCnt, projoect_AttrDesc, &attrDesc[specified_attr_index], op, attrValue, record_length);
}


const Status ScanSelect(const string & result, 
			const int projCnt,
			const AttrDesc projNames[],
			const AttrDesc *attrDesc, 
			const Operator op, 
			const char *filter,
			const int reclen)
{
    cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;

	// 1. Have a temporary record for output table
	char outputData[reclen];

	// 2. Open "result" as an InsertFileScan object
	Status status;
    InsertFileScan resultFile(result, status);
    if (status != OK){
        return status;
	}

	// 3. Open current table (to be scanned) as a HeapFileScan object
    HeapFileScan relFile(projNames[0].relName, status);
    if (status != OK){
        return status;
	}

	//4. check if an unconditional scan is required
	void *realFilter;
    Datatype dt;

	if (filter == nullptr){
		realFilter = NULL;
	}

	//5. check attrType: INTEGER, FLOAT, STRING
	else{
		switch (attrDesc->attrType){
			case INTEGER:{
				dt = INTEGER;
				int iVal = atoi(filter);
				realFilter = &iVal;
				break;
			}
			case FLOAT:{
				dt = FLOAT;
				float fVal = atof(filter);
				realFilter = &fVal;
				break;
			}
			case STRING:{
				dt = STRING;
				realFilter = const_cast<char *>(filter); // No need for conversion
				break;
			}
			default:
				return BADCATPARM; // Invalid attribute type
    	}
	}

	// 6. scan the current table
	status = relFile.startScan(attrDesc->attrOffset, attrDesc->attrLen, dt, (char *)realFilter, op);
    if (status != OK){
        return status;
	}

	// 7. Use while loop to search, if find a record, then copy stuff over to the temporary record
	RID rid;
    Record rec;
    Record resultRec;
    resultRec.data = outputData;
    resultRec.length = reclen;
	RID outRid;
	while (relFile.scanNext(rid) == OK){
		// Get the record
		if ((status = relFile.getRecord(rec)) != OK){
			return status;
		}


		// Create a new record from the projection
		int outputOffset = 0;
		for (int i = 0; i < projCnt; i++){
			memcpy(outputData + outputOffset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
			outputOffset += projNames[i].attrLen;
		}

		// Store the record in the result file
		if ((status = resultFile.insertRecord(resultRec, outRid)) != OK){
			return status;
		}
	}

	// Return OK if the loop completes successfully
    return OK;
}