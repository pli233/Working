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

const Status QU_Select(const string &result,
					   const int projCnt,
					   const attrInfo projNames[],
					   const attrInfo *attr,
					   const Operator op,
					   const char *attrValue)
{
	// QU_Select sets up things and then calls ScanSelect to do the actual work
	cout << "Doing QU_Select " << endl;

	// 1. Validate attribute; only types 0, 1, 2 are allowed
	if ((attr != NULL) && (attr->attrType < 0 || attr->attrType > 2))
	{
		return BADCATPARM;
	}

	// 2. Retrieve information of the attributes found in the relation
	Status status;					 // statue code
	AttrDesc attrDescs[projCnt];	 // Attributes description array
	AttrDesc *specified_attr = NULL; // Pointer to specified attribute
	int record_length = 0;			 // Total Record length

	// 3. Go through the projection list and look up each in the
	// attr cat to get an AttrDesc structure (for offset, length, etc)
	for (int i = 0; i < projCnt; i++)
	{
		// To go from attrInfo to attrDesc, need to consult the catalog (attrCat and relCat, global variables)
		status = attrCat->getInfo(projNames[i].relName, projNames[i].attrName, attrDescs[i]);
		if (status != OK)
		{
			return status;
		}

		// Update record length if no failure
		record_length += attrDescs[i].attrLen;
	}

	// 4. If attr is not NULL, retrieve its information
	if (attr != NULL)
	{
		specified_attr = new AttrDesc;
		status = attrCat->getInfo(attr->relName, attr->attrName, *specified_attr);
		if (status != OK)
		{
			delete specified_attr; // Cleanup if getInfo failed
			return status;
		}
	}

	// 5. Call ScanSelect with the appropriate parameters
	status = ScanSelect(result, projCnt, attrDescs, specified_attr, op, attrValue, record_length);

	// 6 Clean up
	delete specified_attr;
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

	// 1. Create a temporary record for the output table
	outputRec.length = reclen;
	outputRec.data = (char *)malloc(reclen);

	// 2. Open "result" as an InsertFileScan object
	Status status;
	InsertFileScan resultIfs(result, status);
	if (status != OK)
	{
		return status;
	}

	// 3. Open current table (to be scanned) as a HeapFileScan object
	HeapFileScan hfs(projNames[0].relName, status);
	if (status != OK)
	{
		return status;
	}

	// 4. Start a scan; check if an unconditional scan is necessary

	// Careful!! need to declare these value in larger scope so we declare here instead inside of cases
	int intValue;
	float floatValue;

	if (attrDesc == NULL)
	{
		status = hfs.startScan(0, 0, STRING, NULL, (Operator)op); // Use NULL as filter for unconditional search
	}
	else
	{
		// 5. check attrType: INTEGER, FLOAT, STRING
		switch (attrDesc->attrType)
		{
			case STRING:
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, STRING, filter, (Operator)op);
				break;
			case INTEGER:
			{
				intValue = atoi(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char *)&intValue, (Operator)op);
				break;
			}
			case FLOAT:
			{
				floatValue = atof(filter);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char *)&floatValue, (Operator)op);
				break;
			}
		}
	}

	// Check the status of the scan start
	if (status != OK)
	{
		return status;
	}

	// 6. Search using a while loop; copy found records to the temporary record

	RID outRID;
	while (hfs.scanNext(rid) == OK)
	{
		status = hfs.getRecord(rec);
		if (status != OK)
		{
			return status;
		}

		int outputOffset = 0;
		for (int i = 0; i < projCnt; i++)
		{
			memcpy((char *)outputRec.data + outputOffset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
			outputOffset += projNames[i].attrLen;
		}
		// Store the record in the result file
		if ((status = resultIfs.insertRecord(outputRec, outRID)) != OK)
		{
			return status;
		}
	}

	// 7. End the scan and check the status
	status = hfs.endScan();
	if (status != OK)
	{
		return status;
	}
	free(outputRec.data);
	// 8. Completion of all operations
	return OK;
}