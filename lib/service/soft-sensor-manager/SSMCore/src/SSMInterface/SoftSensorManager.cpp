/******************************************************************
*
* Copyright 2014 Samsung Electronics All Rights Reserved.
*
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
******************************************************************/
#include "SSMInterface/SoftSensorManager.h"
#include "QueryProcessor/QueryEngine.h"
#include "QueryProcessor/ContextModel.h"
#include "QueryProcessor/ConditionedModel.h"
#include "QueryProcessor/EvaluationEngine.h"
#include "QueryProcessor/PropagationEngine.h"
#include "SensorProcessor/ResponseReactor.h"
#include "SensorProcessor/ContextExecutor.h"
#include "SensorProcessor/ContextDataReader.h"
#include "SensorProcessor/ResourceFinder.h"
#include "SensorProcessor/ResourceConnectivity.h"

inline bool operator<( const OID & lhs, const OID & rhs )
{
	int ret = memcmp( &lhs, &rhs, sizeof(OID));
	return (ret < 0 ? true : false );
}

SSMRESULT CSoftSensorManager::finalConstruct()
{
	return SSM_S_OK;
}

void CSoftSensorManager::finalRelease()
{
}

SSMRESULT CSoftSensorManager::initializeCore(IN std::string xmlDescription)
{
	SSMRESULT					res = SSM_E_FAIL;
	rapidxml::xml_document<>	xmlDoc;
	std::string					strKey;
	std::string					strValue;
	rapidxml::xml_node<>		*root = NULL;
	rapidxml::xml_node<>		*itemSSMCore = NULL;
	rapidxml::xml_node<>		*itemDevice = NULL;

	std::string					name;
	std::string					type;

	xmlDoc.parse<0>((char *)xmlDescription.c_str());

	root = xmlDoc.first_node();

	strKey = root->name();

	if(strKey != "SSMCore")
	{
		return SSM_E_FAIL;
	}

	for(itemSSMCore = root->first_node(); itemSSMCore; itemSSMCore = itemSSMCore->next_sibling())
	{
		strKey = itemSSMCore->name();

		if(strKey == "Device")
		{
			for(itemDevice = itemSSMCore->first_node(); itemDevice; itemDevice = itemDevice->next_sibling())
			{
				strKey = itemDevice->name();

				if(strKey == "Name")
				{
					name = itemDevice->value();
				}
				else if(strKey == "Type")
				{
					type = itemDevice->value();
				}
				else
				{
					;/*NULL*/
				}
			}
		}
		else
		{
			;/*NULL*/
		}
	}

	SSM_CLEANUP_ASSERT(CreateGlobalInstance(OID_IThreadPool, (IBase**)&m_pThreadPool));
	SSM_CLEANUP_ASSERT(CreateGlobalInstance(OID_IContextRepository, (IBase**)&m_pContextRepository));
	SSM_CLEANUP_ASSERT(CreateGlobalInstance(OID_IResponseReactor, (IBase**)&m_pResponseReactor));
	m_pContextRepository->setCurrentDeviceInfo(name, type);
	
	//SSM_CLEANUP_ASSERT(CreateGlobalInstance(OID_ISharingLayer, (IBase**)&m_pSharingLayer));
	//m_pSharingLayer->InitLayer(m_pLowLevelResponseReactor);
	//m_pSharingLayer->SetLocalId(udn);

	SSM_CLEANUP_ASSERT(CreateGlobalInstance(OID_IPropagationEngine, (IBase**)&m_pPropagationEngine));

CLEANUP:
	if(res != SSM_S_OK)
	{
		terminateCore(false);
	}

	return res;
}

SSMRESULT CSoftSensorManager::startCore()
{
	//m_pSharingLayer->Start();
	return SSM_S_OK;
}

SSMRESULT CSoftSensorManager::stopCore()
{
	//m_pSharingLayer->Stop();
	return SSM_S_OK;
}

SSMRESULT CSoftSensorManager::terminateCore(bool factoryResetFlag)
{
	return SSM_S_OK;
}

SSMRESULT CSoftSensorManager::createQueryEngine(OUT IQueryEngine **ppQueryEngine)
{
	SSMRESULT res = SSM_E_FAIL;
	IQueryEngineInternal	*pQueryEngineInternal = NULL;
	SSM_CLEANUP_ASSERT(CreateInstance(OID_IQueryEngineInternal, (IBase**)&pQueryEngineInternal));
	*ppQueryEngine = (IQueryEngine *)pQueryEngineInternal;

CLEANUP:
	return res;
}

unsigned long CSoftSensorManager::releaseQueryEngine(IN IQueryEngine *pQueryEngine)
{
	IQueryEngineInternal	*pQueryEngineInternal = NULL;
	pQueryEngineInternal = (IQueryEngineInternal *)(CQueryEngine*)pQueryEngine;

	return pQueryEngineInternal->release();
}

SSMRESULT CSoftSensorManager::getInstalledModelList(OUT std::vector<ISSMResource*> *pList)
{
	m_pResponseReactor->getList(pList);

	return SSM_S_OK;
}

CSimpleMutex				*g_mtxGlobalInstance = NULL;
std::map<OID, IBase*>		*g_globalInstance = NULL;

SSMRESULT CreateGlobalInstance(IN const OID& objectID, OUT IBase** ppvObject)
{
	SSMRESULT res = SSM_E_NOINTERFACE;

	if(ppvObject == NULL)
	{
		return SSM_E_POINTER;
	}

	*ppvObject = NULL;

	g_mtxGlobalInstance->lock();
	res = SSM_S_FALSE;
	if(IsEqualOID(OID_IThreadPool, objectID))
	{
		if (g_globalInstance->find(OID_IThreadPool) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IThreadPool, ppvObject));
		}
	}
	else if(IsEqualOID(OID_IEvaluationEngine, objectID))
	{
		if (g_globalInstance->find(OID_IEvaluationEngine) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IEvaluationEngine, ppvObject));
		}
	}
	else if(IsEqualOID(OID_IPropagationEngine, objectID))
	{
		if (g_globalInstance->find(OID_IPropagationEngine) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IPropagationEngine, ppvObject));
		}
	}
	else if(IsEqualOID(OID_IContextRepository, objectID))
	{
		if (g_globalInstance->find(OID_IContextRepository) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IContextRepository, ppvObject));
		}
	}
	else if(IsEqualOID(OID_IContextDataReader, objectID))
	{
		if (g_globalInstance->find(OID_IContextDataReader) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IContextDataReader, ppvObject));
		}
	}
	else if (IsEqualOID(OID_IResponseReactor, objectID))
	{
		if (g_globalInstance->find(OID_IResponseReactor) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IResponseReactor, ppvObject));
		}
	}
	else if (IsEqualOID(OID_IResourceConnectivity, objectID))
	{
		if (g_globalInstance->find(OID_IResourceConnectivity) == g_globalInstance->end())
		{
			SSM_CLEANUP_ASSERT(CreateInstance(OID_IResourceConnectivity, ppvObject));
		}
	}
	else
	{
		res = SSM_E_NOINTERFACE;
	}	

	switch(res)
	{
	case SSM_S_OK:
		(*g_globalInstance)[objectID] = *ppvObject;
		break;

	case SSM_S_FALSE:
		(*g_globalInstance)[objectID]->addRef();
		*ppvObject = (*g_globalInstance)[objectID];
		res = SSM_S_OK;
		break;

	default:
		goto CLEANUP;
	}

CLEANUP:
	g_mtxGlobalInstance->unlock();
	return res;
}

SSMRESULT CreateInstance(IN const OID& objectID, OUT IBase** ppObject)
{
	SSMRESULT res = SSM_E_NOINTERFACE;

	if(ppObject == NULL)
	{
		return SSM_E_POINTER;
	}

	*ppObject = NULL;

	if(IsEqualOID(OID_IThreadPool, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CThreadPool>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IEvaluationEngine, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CEvaluationEngine>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IPropagationEngine, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CPropagationEngine>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IContextRepository, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CContextRepository>(objectID, ppObject));
	}
	else if (IsEqualOID(OID_IResponseReactor, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CResponseReactor>(objectID, ppObject));
	}
	else if (IsEqualOID(OID_IContextExecutor, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CContextExecutor>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IContextModel, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CContextModel>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IConditionedModel, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CConditionedModel>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IConditionedQueryResult, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CConditionedQueryResult>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IConditionedQuery, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CConditionedQuery>(objectID, ppObject));
	}
	else if (IsEqualOID(OID_IResourceFinder, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CResourceFinder>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IQueryEngineInternal, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CQueryEngine>(objectID, ppObject));
	}
	else if (IsEqualOID(OID_ISoftSensorManager, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CSoftSensorManager>(objectID, ppObject));
	}
	else if(IsEqualOID(OID_IContextDataReader, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CContextDataReader>(objectID, ppObject));
	}
	else if (IsEqualOID(OID_IResourceConnectivity, objectID))
	{
		SSM_CLEANUP_ASSERT(CreateNewObject<CResourceConnectivity>(objectID, ppObject));
	}

CLEANUP:
	return res;
}

SSMRESULT CreateGlobalInstanceRepo()
{
	SSMRESULT res = SSM_E_FAIL;

	if (g_mtxGlobalInstance != NULL)
		SSM_CLEANUP_ASSERT(SSM_E_OUTOFMEMORY);

	if (g_globalInstance != NULL)
		SSM_CLEANUP_ASSERT(SSM_E_OUTOFMEMORY);

	g_mtxGlobalInstance = new CSimpleMutex();
	SSM_CLEANUP_NULL_ASSERT(g_mtxGlobalInstance);
	g_globalInstance = new std::map<OID, IBase*>();
	SSM_CLEANUP_NULL_ASSERT(g_globalInstance);

	res = SSM_S_OK;

CLEANUP:
	return res;
}

SSMRESULT DestroyGlobalInstanceRepo()
{
	SAFE_DELETE(g_mtxGlobalInstance);
	SAFE_DELETE(g_globalInstance);
	return SSM_S_OK;
}
