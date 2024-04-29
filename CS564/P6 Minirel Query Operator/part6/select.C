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


    Status status;
    RID rid;
    Record rec;
    int i;
    Datatype t;

    Record resultRec;
    char outputData[reclen];
    resultRec.data = (void *)outputData;
    resultRec.length = reclen;
    int outputOffset;

    // 打开关系文件以读取，并打开结果文件以写入
    InsertFileScan resultFile(result, status);
    if (status != OK)
        return status;
    HeapFileScan relFile(projNames[0].relName, status);
    if (status != OK)
        return status;

    void *realFilter;

    // 设置扫描条件
    if (filter != NULL)
    {
        // 如果给定了特定的筛选器，执行可能的数据转换
        switch (attrDesc->attrType)
        {
        case 1:
        {
            t = INTEGER;
            int iVal = atoi(filter);
            realFilter = &iVal;
            break;
        }
        case 2:
        {
            t = FLOAT;
            float fVal = atof(filter);
            realFilter = &fVal;
            break;
        }
        case 0:
        {
            t = STRING;
            realFilter = (void *)filter;
            break;
        }
        }
        status = relFile.startScan(attrDesc->attrOffset, attrDesc->attrLen, t, (char *)realFilter, op);
    }
    else
    {
        // 没有给定特定的筛选器；设置为顺序扫描
        status = relFile.startScan(attrDesc->attrOffset, attrDesc->attrLen, t, NULL, op);
    }

    if (status != OK)
        return status;

    while (relFile.scanNext(rid) == OK)
    {
        // 当前记录符合搜索条件

        // 获取记录
        if ((status = relFile.getRecord(rec)) != OK)
            return status;

        // 1) 从投影中创建新记录
        outputOffset = 0;
        for (i = 0; i < projCnt; i++)
        {
            memcpy(outputData + outputOffset, (char *)rec.data + projNames[i].attrOffset, projNames[i].attrLen);
            outputOffset += projNames[i].attrLen;
        }

        // 2) 存储到结果文件中
        RID outRID;
        status = resultFile.insertRecord(resultRec, outRID);
        if (status != OK)
            return status;
    }

    return OK;
}