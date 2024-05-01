#include <stdlib.h>
#include "catalog.h"
#include "query.h"

// This function will delete all tuples in relation satisfying the predicate specified by attrName,
//  op, and the constant attrValue. type denotes the type of the attribute. You can locate all the qualifying tuples using a filtered HeapFileScan.
const Status QU_Delete(const string &relation,
					   const string &attrName,
					   const Operator op,
					   const Datatype type,
					   const char *attrValue)

{

	// 1. Validate relation name
	if (relation.empty())
	{
		return BADCATPARM;
	}

	// 2. Create hfs, relevent and check status
	Status status;
	RID rid;
	AttrDesc *attrDesc = new AttrDesc;
	HeapFileScan hfs(relation, status);

	// Abort if the initial file scan setup fails.
	if (status != OK)
	{
		return status;
	}

	// 3. Start a scan; check if an unconditional scan is necessary

	// Careful!! need to declare these value in larger scope so we declare here instead inside of cases
	int intVal;
	float floatVal;
	if (attrName.empty())
	{
		// Use unconditional scan
		status = hfs.startScan(0, 0, STRING, NULL, op);
	}
	else
	{
		// getting information about attribute
		status = attrCat->getInfo(relation, attrName, *attrDesc);
		if (status != OK)
		{
			return status;
		}

		// 4. check attrType: INTEGER, FLOAT, STRING
		switch (type)
		{
			case INTEGER:
			{
				intVal = atoi(attrValue);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, INTEGER, (char *)&intVal, op);
				break;
			}

			case FLOAT:
			{
				floatVal = atof(attrValue);
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, FLOAT, (char *)&floatVal, op);
				break;
			}
			case STRING:
			{
				status = hfs.startScan(attrDesc->attrOffset, attrDesc->attrLen, STRING, attrValue, op);
				break;
			}
		}
	}

	// check start scan status
	if (status != OK)
	{
		return status;
	}

	// 5. Iterate over all records that meet the condition and delete them.
	while ((status = hfs.scanNext(rid)) == OK)
	{
		status = hfs.deleteRecord();
		if (status != OK)
		{
			return status;
		}
	}

	if (status != FILEEOF)
	{
		return status;
	}

	// 6. Close the scan once complete.
	hfs.endScan();

	return OK;
}