#include "catalog.h"
#include "query.h"
#include "stdio.h"
#include "stdlib.h"

// forward declaration
const Status ScanSelect(const string &result,
						const int projCnt,
						const AttrDesc projNames[],
						const AttrDesc *attrDesc,
						const Operator op,
						const char *filter,
						const int reclen);

/*
 * Selects records from the specified relation.

 * @param result - result of selection
 * @param projCnt -
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Select(const string &result,
					   const int projCnt,
					   const attrInfo projNames[],
					   const attrInfo *attr,
					   const Operator op,
					   const char *attrValue)
{
	// QU_Select sets up things and then calls ScanSelect to do the actual work
	cout << "Doing QU_Select " << endl;

	Status status;
	AttrDesc attrDescArray[projCnt]; // holds desired projection
	AttrDesc *attrDesc = NULL;		 // desired search value
	int reclen = 0;

	// go through the projection list and look up each in the
	// attr cat to get an AttrDesc structure (for offset, length, etc)
	for (int i = 0; i < projCnt; i++)
	{
		status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, attrDescArray[i]);
		if (status != OK)
		{
			return status;
		}
	}

	// if attr is not NULL, get its info
	if (attr != NULL)
	{
		attrDesc = new AttrDesc;
		status = attrCat->getInfo(attr->relName, attr->attrName, *attrDesc);
		if (status != OK)
		{
			delete attrDesc; // Cleanup if getInfo failed
			return status;
		}
	}

	// get output record length from attrdesc structures
	for (int i = 0; i < projCnt; i++)
	{
		reclen += attrDescArray[i].attrLen;
	}

	// Call ScanSelect
	status = ScanSelect(result, projCnt, attrDescArray, attrDesc, op, attrValue, reclen);

	// Cleanup
	delete attrDesc; // Cleanup if attrDesc was allocated

	return status;
}

const Status ScanSelect(const string &result,
						const int projCnt,
						const AttrDesc projNames[],
						const AttrDesc *attrDesc,
						const Operator op,
						const char *filter,
						const int reclen)
{
	cout << "Doing HeapFileScan Selection using ScanSelect()" << endl;

	Status status;
	Record outputRec;
	RID rid;
	Record rec;

	InsertFileScan resultRel(result, status);
	if (status != OK)
	{
		return status;
	}

	outputRec.length = reclen;
	outputRec.data = (char *)malloc(reclen);

	// start scan
	cout << "Starting Scan" << endl; // debugging
	HeapFileScan scan(projNames[0].relName, status);
	if (status != OK)
	{
		return status;
	}
	cout << "Check Type" << endl; // debugging
	// start scan for different data types
	int toInt;
	float toFloat;
	if (attrDesc == NULL)
	{
		status = scan.startScan(0, 0, STRING, NULL, EQ);
	}
	else if (attrDesc->attrType == STRING)
	{
		status = scan.startScan(attrDesc->attrOffset, attrDesc->attrLen, STRING, filter, op);
	}
	else if (attrDesc->attrType == FLOAT)
	{
		toFloat = atof(filter);
		status = scan.startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char *)&toFloat, op);
	}
	else if (attrDesc->attrType == INTEGER)
	{
		toInt = atoi(filter);
		status = scan.startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char *)&toInt, op);
	}

	// check if startScan works as expected
	if (status != OK)
	{
		return status;
	}

	cout << "scanning..." << endl; // debugging
	while (scan.scanNext(rid) == OK)
	{
		status = scan.getRecord(rec);
		if (status != OK)
		{
			return status;
		}

		int outputOffset = 0;
		for (int i = 0; i < projCnt; i++){
			memcpy(outputRec.data + outputOffset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
			outputOffset += projNames[i].attrLen;
		}


		RID outRID;
		// Store the record in the result file
		if ((status = resultRel.insertRecord(outputRec, outRID)) != OK){
			return status;
		}		#include "catalog.h"
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
    HeapFileScan hfs(projNames[0].relName, status);
    if (status != OK){
        return status;
	}

	//4. check if an unconditional scan is required

	if (attrDesc == NULL) {
		status = hfs.startScan(0, 0, STRING,  NULL, EQ); // 使用 NULL 作为 filter，并设定 op 为 EQ
	}
	//5. check attrType: INTEGER, FLOAT, STRING
	else {
		switch (attrDesc->attrType) {
			case STRING:
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, STRING, filter, (Operator)op); // 使用字符串作为 filter，并将 op 转换为 Operator 类型
				break;
			case INTEGER: {
				int intValue = atoi(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char*)&intValue, (Operator)op); // 使用整数作为 filter，并将 op 转换为 Operator 类型
				break;
			}
			case FLOAT: {
				float floatValue = atof(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char*)&floatValue, (Operator)op); // 使用浮点数作为 filter，并将 op 转换为 Operator 类型
				break;
			}
		}
	}

	// 6. scan the current table
	if (status != OK) {
		return status;
	}



	// 7. Use while loop to search, if find a record, then copy stuff over to the temporary record
	RID rid;
    Record rec;
    Record resultRec;
    resultRec.data = (void *)outputData;
    resultRec.length = reclen;
	RID outRid;
	while (hfs.scanNext(rid) == OK){
		// Get the record
		if ((status = hfs.getRecord(rec)) != OK){
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
	}

	cout << "Scan finished" << endl; // debugging
	status = scan.endScan();
	if (status != OK)
	{
		return status;
	}
	return OK;
}