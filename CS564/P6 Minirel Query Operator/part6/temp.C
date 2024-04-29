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