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
#ifndef _SSMCore_H_
#define _SSMCore_H_

#include <string>
#include <vector>

#include "SSMModelDefinition.h"

#define IN
#define OUT
/*
#if defined(WIN32) && defined(SSMCORE_WINDOWS_EXPORTS)
#define INTERFACE_DECLSPEC    __declspec(dllexport)
#elif defined(WIN32)
#define INTERFACE_DECLSPEC    __declspec(dllimport)
#pragma comment(lib, "SSMCore_Windows.lib")
#else
#define INTERFACE_DECLSPEC
#endif
*/
#define INTERFACE_DECLSPEC

enum SSMRESULT
{
	SSM_S_OK
	, SSM_S_FALSE
	, SSM_E_POINTER
	, SSM_E_OUTOFMEMORY
	, SSM_E_FAIL
	, SSM_E_NOINTERFACE
	, SSM_E_NOTIMPL
};

/**
* @class    IModelData
* @brief    IModelData Interface
*			 This class represents context model data package
*
* @see
*/
class IModelData
{
public:
	/**
	* @fn	  getDataId
	* @brief Get affected DataId. ContextModel has plenty of data so \n
	*		  returned data is matched from given condition
	*
	* @param None
	* 
	* @return int
	* @warning      
	* @exception    
	* @see 
	*/
	virtual int getDataId() = 0;

	/**
	* @fn	  GetPropertyCount
	* @brief ContextModel has at least one property that contains data \n
	*		  property is described from its specification.
	*
	* @param None
	* 
	* @return int
	* @warning      
	* @exception    
	* @see 
	*/
	virtual int getPropertyCount() = 0;

	/**
	* @fn	  getPropertyName
	* @brief Retrieve propertyName
	*
	* @param [in] int propertyIndex - index of property to read
	* 
	* @return std::string
	* @warning      
	* @exception    
	* @see 
	*/
	virtual std::string getPropertyName(IN int propertyIndex) = 0;

	/**
	* @fn	  getPropertyValue
	* @brief Retrieve propertyValue
	*
	* @param [in] int propertyIndex - index of property to read
	* 
	* @return std::string
	* @warning      
	* @exception    
	* @see
	*/
	virtual std::string getPropertyValue(IN int propertyIndex) = 0;

	/**
	* @fn	  getPropertyValueByName
	* @brief Retrieve propertyValue using given name
	*
	* @param [in] std::string propertyName - property name looking for
	* 
	* @return std::string
	* @warning      
	* @exception    
	* @see
	*/
	virtual std::string getPropertyValueByName(IN std::string propertyName) = 0;
protected:
	virtual ~IModelData(){};
};

/**
* @class    IDataReader
* @brief    IDataReader Interface
*			 This class represents context model data package's reader
*
* @see
*/
class IDataReader
{
public:
	/**
	* @fn	  getAffectedModels
	* @brief Get affected ContextModels. The CQL can specify multiple ContextModels for retrieving data.
	*
	* @param [out] std::vector<std::string> *pAffectedModels - affected ContextModel list
	* 
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT getAffectedModels(OUT std::vector<std::string> *pAffectedModels) = 0;

	/**
	* @fn	  getModelDataCount
	* @brief Get affected data count. There are multiple data can exist from given condition.
	*
	* @param [in] std::string modelName - affected ContextModel name
	*
	* @param [out] int *pDataCount - affected dataId count
	* 
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT getModelDataCount(IN std::string modelName, OUT int *pDataCount) = 0;

	/**
	* @fn	  getModelData
	* @brief Get actual Context Model data
	*
	* @param [in] std::string modelName - affected ContextModel name
	*				
	*
	* @param [in] int dataIndex - affected dataId index
	*				
	* 
	* @param [out] IModelData **ppModelData - affected ContextModel data reader
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT getModelData(IN std::string modelName, IN int dataIndex, OUT IModelData **ppModelData) = 0;
protected:
	virtual ~IDataReader(){};
};

/**
* @class    IQueryEngineEvent
* @brief    IQueryEngineEvent Interface
*			 This class represents Query Engine's event that contains results
*
* @see
*/
class IQueryEngineEvent
{
public:
	/**
	* @fn	  onQueryEngineEvent
	* @brief Transmit result of SSMCore to Application layer
	*
	* @param [in] int cqid - entered ContextQuery ID
	*            
	* @param [in] IDataReader *pResult - result of SSMCore
	*            
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT onQueryEngineEvent(IN int cqid, IN IDataReader *pResult) = 0;
protected:
	virtual ~IQueryEngineEvent(){};
};

/**
* @class    IQueryEngine
* @brief    IQueryEngine Interface
*			 This class represents main interface of Query Engine
*
* @see
*/
class IQueryEngine
{
public:
	/**
	* @fn	  executeContextQuery
	* @brief Execute ContextQuery and return ContextQuery ID
	*
	* @param [in] std::string ContextQuery - Entered ContetxQuery
	*
	* @param [out] int *cqid - ID of ContextQuery
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT executeContextQuery(IN std::string contextQuery, OUT int *cqid) = 0;

	/**
	* @fn	  registerQueryEvent
	* @brief Register QueryEngineEvent to QueryEngine.
	*
	* @param [in] IQueryEngineEvent *pQueryEngineEvent - Register QueryEngineEvent
	*            
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT registerQueryEvent(IN IQueryEngineEvent *pQueryEngineEvent) = 0;

	/**
	* @fn	  unregisterQueryEvent
	* @brief Unregister QueryEngineEvent to QueryEngine.
	*
	* @param [in] IQueryEngineEvent *pQueryEngineEvent - Unregister QueryEngineEvent
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT unregisterQueryEvent(IN IQueryEngineEvent *pQueryEngineEvent) = 0;

	/**
	* @fn	 killContextQuery
	* @brief Kill registered ContextQuery according to cqid
	*
	* @param [in] int cqid - Context query corresponding to the cqid will be terminated
	*           
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	virtual SSMRESULT killContextQuery(IN int cqid) = 0;
protected:
	virtual ~IQueryEngine(){};
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

	/**
	* @fn	 CreateQueryEngine
	* @brief Create QueryEngine instance.
	*
	* @param [out] IQueryEngine **ppQueryEngine - address of QueryEngine
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT CreateQueryEngine(OUT IQueryEngine **ppQueryEngine);

	/**
	* @fn	 ReleaseQueryEngine
	* @brief Release QueryEngine instance.
	*
	* @param [in] IQueryEngine *pQueryEngine - Address of QueryEngine
	*
	* @return unsigned long
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC unsigned long ReleaseQueryEngine(IN IQueryEngine *pQueryEngine);


	/**
	* @fn	 InitializeSSMCore
	* @brief Initialize framework using given configuration
	*
	* @param [in] std::string xmlDescription - Framework specification described in XML
	*           
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT InitializeSSMCore(IN std::string xmlDescription);

	/**
	* @fn	 StartSSMCore
	* @brief Start framework that allows other devices discover and communication
	*
	* @param None
	* 
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT StartSSMCore();

	/**
	* @fn	 StopSSMCore
	* @brief Stop framework
	*
	* @param None
	* 
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT StopSSMCore();

	/**
	* @fn	 TerminateSSMCore
	* @brief Terminate framework, return all allocated resources
	*
	* @param [in] bool factoryResetFlag - Set true if framework needs to reset
	*            
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT TerminateSSMCore(IN bool factoryResetFlag = false);

	/**
	* @fn	 GetErrorMessage
	* @brief Prints Error message from SSMRESULT error code
	*
	* @param [in] SSMRESULT res - return code
	*
	* @return const char *
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC const char *GetSSMError(IN SSMRESULT res);

	/**
	* @fn	 GetInstalledModelList
	* @brief Gets all installed models from local and remote
	*
	* @param [out] std::vector<ISSMResource> *pList - List of installed context model
	*
	* @return SSMRESULT
	* @warning      
	* @exception    
	* @see
	*/
	INTERFACE_DECLSPEC SSMRESULT GetInstalledModelList(OUT std::vector<ISSMResource*> *pList);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
