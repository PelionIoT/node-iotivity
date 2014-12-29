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
#include "SSMInterface/SSMCore_JNI.h"
#include "Common/PlatformLayer.h"

#define JNI_E_INVALIDARG -1001
#define JNI_E_METHODID   -1002

JavaVM *g_JVM = NULL;

#if defined(WIN32)
#define ATTACH_CURRENT_THREAD(javaVM, env) if(env == NULL) javaVM->AttachCurrentThread((void**)&env, NULL)
#else
#define ATTACH_CURRENT_THREAD(javaVM, env) if(env == NULL) javaVM->AttachCurrentThread(&env, NULL)
#endif
#define DETACH_CURRENT_THREAD(javaVM) javaVM->DetachCurrentThread()

jclass						g_ClassQueryEngine = NULL;
jclass						g_ClassDataReader = NULL;
jclass						g_ClassModelData = NULL;
jclass						g_ClassQueryEngineEvent = NULL;

jclass						g_ClassReportReceiver = NULL;
jobject						g_objReportReceiver = NULL;

void ReportMessage(const char *tag, const char *msg)
{
	JNIEnv *env;

	if(g_objReportReceiver == NULL)
		return;

	g_JVM->GetEnv((void**) &env, JNI_VERSION_1_6);

	ATTACH_CURRENT_THREAD(g_JVM, env);

	jmethodID midReportReceiver = 
		env->GetMethodID(g_ClassReportReceiver, "OnMessageReceived", "(Ljava/lang/String;Ljava/lang/String;)V");

	jclass nonVIrtualObject = env->GetObjectClass(g_objReportReceiver);

	env->CallNonvirtualVoidMethod(g_objReportReceiver, nonVIrtualObject, midReportReceiver, env->NewStringUTF(tag), env->NewStringUTF(msg));
	
	DETACH_CURRENT_THREAD(g_JVM);

	return;
}

class QueryEngineEventReceiver 
	: public IQueryEngineEvent
{
private:
	jobject			m_objQueryEngineEvent;

public:
	QueryEngineEventReceiver()
	{
		m_objQueryEngineEvent = NULL;
	}

	~QueryEngineEventReceiver()
	{
		if(m_objQueryEngineEvent != NULL)
		{
			JNIEnv *env;
			g_JVM->GetEnv((void**) &env, JNI_VERSION_1_6);
			env->DeleteGlobalRef(m_objQueryEngineEvent);
		}
	}

	void SetQueryEngineEventObj(jobject objQueryEngineEvent)
	{
		if(m_objQueryEngineEvent != NULL)
		{
			JNIEnv *env;
			g_JVM->GetEnv((void**) &env, JNI_VERSION_1_6);
			env->DeleteGlobalRef(m_objQueryEngineEvent);
		}

		m_objQueryEngineEvent = objQueryEngineEvent;
	}

	SSMRESULT onQueryEngineEvent(IN int cqid, IN IDataReader *pResult)
	{
		JNIEnv *env;
		g_JVM->GetEnv((void**) &env, JNI_VERSION_1_6);

		ATTACH_CURRENT_THREAD(g_JVM, env);

		jmethodID midQueryEngineEvent = 
			env->GetMethodID(g_ClassQueryEngineEvent, "OnQueryEngineEvent", "(ILcom/sec/android/ssmcore/DataReader;)V");

		jclass nonVIrtualObject = env->GetObjectClass(m_objQueryEngineEvent);

		jmethodID cid_DataReader = env->GetMethodID(g_ClassDataReader, "<init>", "(I)V");

		if(cid_DataReader == NULL)
			return SSM_E_FAIL;

		env->CallNonvirtualVoidMethod(m_objQueryEngineEvent, nonVIrtualObject, midQueryEngineEvent, (jint)cqid, 
			env->NewObject(g_ClassDataReader, cid_DataReader, (jint)pResult));

		DETACH_CURRENT_THREAD(g_JVM);

		return SSM_S_OK;
	}
};

QueryEngineEventReceiver	*g_QueryEngineEventReceiver = NULL;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* jvm, void* reserved)
{
	JNIEnv *env;
	if (jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
		return JNI_ERR;
		
	g_ClassQueryEngine = (jclass) env->NewGlobalRef(env->FindClass("com/sec/android/ssmcore/QueryEngine"));

	g_ClassDataReader = (jclass) env->NewGlobalRef(env->FindClass("com/sec/android/ssmcore/DataReader"));

	g_ClassModelData = (jclass) env->NewGlobalRef(env->FindClass("com/sec/android/ssmcore/ModelData"));

	g_ClassQueryEngineEvent = (jclass) env->NewGlobalRef(env->FindClass("com/sec/android/ssmcore/IQueryEngineEvent"));


	g_ClassReportReceiver = (jclass) env->NewGlobalRef(env->FindClass("com/sec/android/ssmcore/IReportReceiver"));


	g_JVM = jvm;

	g_QueryEngineEventReceiver = new QueryEngineEventReceiver();

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* jvm, void* reserved) 
{
	JNIEnv *env;
	if (jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
		return;

	if(g_ClassQueryEngine != NULL)
		env->DeleteGlobalRef(g_ClassQueryEngine);

	if(g_ClassDataReader != NULL)
		env->DeleteGlobalRef(g_ClassDataReader);

	if(g_ClassModelData != NULL)
		env->DeleteGlobalRef(g_ClassModelData);

	if(g_ClassQueryEngineEvent != NULL)
		env->DeleteGlobalRef(g_ClassQueryEngineEvent);


	if(g_ClassReportReceiver != NULL)
		env->DeleteGlobalRef(g_ClassReportReceiver);

	if(g_objReportReceiver != NULL)
		env->DeleteGlobalRef(g_objReportReceiver);
	

	g_JVM = NULL;

	if(g_QueryEngineEventReceiver != NULL)
		delete g_QueryEngineEventReceiver;
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_InitializeSSMCore
	(JNIEnv *env, jclass clz, jstring jstrXmlDescription)
{
	SSMRESULT res = SSM_E_FAIL;
	const char *xmlDescription = env->GetStringUTFChars(jstrXmlDescription, NULL);

	res = InitializeSSMCore(xmlDescription);

	env->ReleaseStringUTFChars(jstrXmlDescription, xmlDescription);

	if(res != SSM_S_OK)
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "InitializeSSMCore failed");
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_StartSSMCore
	(JNIEnv *env, jclass clz)
{
	if(StartSSMCore() != SSM_S_OK)
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "StartSSMCore failed");
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_StopSSMCore
	(JNIEnv *env, jclass clz)
{
	if(StopSSMCore() != SSM_S_OK)
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "StopSSMCore failed");
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_TerminateSSMCore
	(JNIEnv *env, jclass clz)
{
	if(TerminateSSMCore() != SSM_S_OK)
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "TerminateSSMCore failed");
}

JNIEXPORT jobject JNICALL Java_com_sec_android_ssmcore_CoreController_CreateQueryEngine
	(JNIEnv *env, jclass clz)
{
	IQueryEngine		*pQueryEngine = NULL;

	if(CreateQueryEngine(&pQueryEngine) != SSM_S_OK)	
	{
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "CreateQueryEngine failed");
		return NULL;
	}

	jmethodID cid_QueryEngine = env->GetMethodID(g_ClassQueryEngine, "<init>", "(I)V");

	if(cid_QueryEngine == NULL)
		return NULL;

	return env->NewObject(g_ClassQueryEngine, cid_QueryEngine, (jint)pQueryEngine);
}

JNIEXPORT jint JNICALL Java_com_sec_android_ssmcore_CoreController_ReleaseQueryEngine
	(JNIEnv *env, jclass clz, jobject queryEngine)
{
	IQueryEngine		*pQueryEngine = NULL;

	jmethodID mid_GetQueryEngineInstance = env->GetMethodID(g_ClassQueryEngine, "GetQueryEngineInstance", "()I");

	if(mid_GetQueryEngineInstance == NULL)
		return JNI_E_METHODID;

	pQueryEngine = (IQueryEngine*)env->CallIntMethod(queryEngine, mid_GetQueryEngineInstance);

	return ReleaseQueryEngine(pQueryEngine);
}

JNIEXPORT jint JNICALL Java_com_sec_android_ssmcore_CoreController_ExecuteContextQuery
	(JNIEnv *env, jclass clz, jint pQueryEngineInstance, jstring jstrContextQuery)
{
	int					cqid = 0;
	IQueryEngine		*pQueryEngine = (IQueryEngine*)pQueryEngineInstance;

	const char *contextQuery = env->GetStringUTFChars(jstrContextQuery, NULL);

	if(pQueryEngine->executeContextQuery(contextQuery, &cqid) != SSM_S_OK)
	{
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "ExecuteContextQuery failed");
	}

	env->ReleaseStringUTFChars(jstrContextQuery, contextQuery);
	return cqid;
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_RegisterQueryEvent
	(JNIEnv *env, jclass clz, jint pQueryEngineInstance, jobject queryEngineEvent)
{
	IQueryEngine		*pQueryEngine = (IQueryEngine*)pQueryEngineInstance;

	if(queryEngineEvent == NULL)
	{
		pQueryEngine->unregisterQueryEvent(g_QueryEngineEventReceiver);
		return;
	}

	g_QueryEngineEventReceiver->SetQueryEngineEventObj(env->NewGlobalRef(queryEngineEvent));
	pQueryEngine->registerQueryEvent(g_QueryEngineEventReceiver);
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_KillContextQuery
	(JNIEnv *env, jclass clz, jint pQueryEngineInstance, jint cqid)
{
	IQueryEngine		*pQueryEngine = (IQueryEngine*)pQueryEngineInstance;

	if(pQueryEngine->killContextQuery(cqid) != SSM_S_OK)
	{
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "KillContextQuery failed");
	}
}

JNIEXPORT jint JNICALL Java_com_sec_android_ssmcore_CoreController_GetDataId
	(JNIEnv *env, jclass clz, jint pDataReaderInstance)
{
	IModelData *pDataReader = (IModelData *)pDataReaderInstance;

	return pDataReader->getDataId();
}

JNIEXPORT jint JNICALL Java_com_sec_android_ssmcore_CoreController_GetPropertyCount
	(JNIEnv *env, jclass clz, jint pIModelDataInstance )
{
	IModelData *pModelData = (IModelData *)pIModelDataInstance;

	return pModelData->getPropertyCount();
}

JNIEXPORT jstring JNICALL Java_com_sec_android_ssmcore_CoreController_GetPropertyName
	(JNIEnv *env, jclass clz, jint pIModelDataInstance, jint propertyIndex )
{
	IModelData *pModelData = (IModelData *)pIModelDataInstance;

	return env->NewStringUTF(pModelData->getPropertyName(propertyIndex).c_str());
}

JNIEXPORT jstring JNICALL Java_com_sec_android_ssmcore_CoreController_GetPropertyValue
	(JNIEnv *env, jclass clz, jint pIModelDataInstance, jint propertyIndex )
{
	IModelData *pModelData = (IModelData *)pIModelDataInstance;

	return env->NewStringUTF(pModelData->getPropertyValue(propertyIndex).c_str());
}

JNIEXPORT jobject JNICALL Java_com_sec_android_ssmcore_CoreController_GetAffectedModels
	(JNIEnv *env, jclass clz, jint pDataReaderInstance)
{
	IDataReader *pDataReader = (IDataReader *)pDataReaderInstance;
	std::vector<std::string>	affectedModels;

	jclass listClass = env->FindClass("java/util/ArrayList");

	jmethodID add_mid = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");

	jmethodID list_cid = env->GetMethodID(listClass, "<init>", "()V");

	jobject objAffectedModels = env->NewObject(listClass, list_cid);

	pDataReader->getAffectedModels(&affectedModels);

	for(std::vector<std::string>::iterator itor = affectedModels.begin();
		itor != affectedModels.end(); ++itor)
	{
		env->CallBooleanMethod(objAffectedModels, add_mid, env->NewStringUTF((*itor).c_str()));
	}

	return objAffectedModels;
}

JNIEXPORT jint JNICALL Java_com_sec_android_ssmcore_CoreController_GetModelDataCount
	(JNIEnv *env, jclass clz, jint pDataReaderInstance, jstring jstrModelName)
{
	IDataReader *pDataReader = (IDataReader *)pDataReaderInstance;
	int modelCount = 0;
	const char *modelName = env->GetStringUTFChars(jstrModelName, NULL);
	
	if(pDataReader->getModelDataCount(modelName,&modelCount) != SSM_S_OK)
	{
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "GetModelDataCount failed");
	}

	env->ReleaseStringUTFChars(jstrModelName, modelName);

	return modelCount;
}
//return IModelData
JNIEXPORT jobject JNICALL Java_com_sec_android_ssmcore_CoreController_GetModelData
	(JNIEnv *env, jclass clz, jint pDataReaderInstance, jstring jstrModelName, jint jintDataIndex )
{
	IDataReader *pDataReader = (IDataReader *)pDataReaderInstance;
	IModelData *pModelData = NULL;
	const char *modelName = env->GetStringUTFChars(jstrModelName, NULL);

	if(pDataReader->getModelData(modelName,jintDataIndex,&pModelData) != SSM_S_OK)
	{
		env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "GetModelData failed");
		return NULL;
	}
	
	jmethodID cid_ModelData = env->GetMethodID(g_ClassModelData, "<init>", "(I)V");

	if(cid_ModelData == NULL)
		return NULL;

	return env->NewObject(g_ClassModelData, cid_ModelData, (jint)pModelData);
}

JNIEXPORT void JNICALL Java_com_sec_android_ssmcore_CoreController_RegisterReportReceiver
	(JNIEnv *env, jclass clz, jobject reportReceiver)
{
	if(g_objReportReceiver != NULL)
		env->DeleteGlobalRef(g_objReportReceiver);

	g_objReportReceiver = NULL;

	if(reportReceiver != NULL)
		g_objReportReceiver = env->NewGlobalRef(reportReceiver);
}
