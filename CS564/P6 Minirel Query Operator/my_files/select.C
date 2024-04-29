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


	Record outputRec;
	RID rid;
	Record rec;


	// 1. Have a temporary record for output table
	outputRec.length = reclen;
	outputRec.data = (char *)malloc(reclen);

	// 2. Open "result" as an InsertFileScan object
	Status status;
	InsertFileScan resultRel(result, status);
	if (status != OK){
		return status;
	}

	// 3. Open current table (to be scanned) as a HeapFileScan object
    HeapFileScan hfs(projNames[0].relName, status);
    if (status != OK){
        return status;
	}

	//4. check if an unconditional scan is required

	//Careful!! need to declare these value in larger scope so we declare here instead inside of cases
	int intValue; 
	float floatValue; 
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
				intValue = atoi(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char*)&intValue, (Operator)op); // 使用整数作为 filter，并将 op 转换为 Operator 类型
				break;
			}
			case FLOAT: {
				floatValue = atof(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char*)&floatValue, (Operator)op); // 使用浮点数作为 filter，并将 op 转换为 Operator 类型
				break;
			}
		}
	}

	//check the start scan of hfs successfully
	if (status != OK){
		return status;
	}

	//6. Use while loop to search, if find a record, then copy stuff over to the temporary record
	while (hfs.scanNext(rid) == OK)
	{
		status = hfs.getRecord(rec);
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
		}		
	}

	//7. Enc Scan and check status
	status = hfs.endScan();
	if (status != OK){
		return status;
	}

	//8. everything done
	return OK;
}