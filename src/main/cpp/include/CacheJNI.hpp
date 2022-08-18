#include <jni.h>

//
// Created by 叶永平 on 2022/8/9.
//

#ifndef HCACHE_CACHEJNI_H
#define HCACHE_CACHEJNI_H

#include <jni.h>
#include "KVSingleton.hpp"
#include <jni.h>
#include <jni.h>
#include <jni.h>

extern "C" {
std::string jstring2string(JNIEnv *env, jstring jStr) {
    if (!jStr)
        return "";

    const jclass stringClass = env->GetObjectClass(jStr);
    const jmethodID getBytes = env->GetMethodID(stringClass, "getBytes", "(Ljava/lang/String;)[B");
    const jbyteArray stringJbytes = (jbyteArray) env->CallObjectMethod(jStr, getBytes, env->NewStringUTF("UTF-8"));

    size_t length = (size_t) env->GetArrayLength(stringJbytes);
    jbyte* pBytes = env->GetByteArrayElements(stringJbytes, NULL);

    std::string ret = std::string((char *)pBytes, length);
    env->ReleaseByteArrayElements(stringJbytes, pBytes, JNI_ABORT);

    env->DeleteLocalRef(stringJbytes);
    env->DeleteLocalRef(stringClass);
    return ret;
}

cache::DiskKVStorage * getKV(JNIEnv *env, jobject thiz) {
    const jclass obj = env->GetObjectClass(thiz);
    const jfieldID pathID = env->GetFieldID(obj, "path", "Ljava/lang/String;");
    const jstring path = (jstring)env->GetObjectField(thiz, pathID);
    jboolean isCopy;
    const char *_path = env->GetStringUTFChars(path, &isCopy);
    if (_path == NULL) {
        env->DeleteLocalRef(obj);
        return nullptr;
    } else {
        auto kv = cache::KVSingleton::getInstance().getKV(_path);
        env->DeleteLocalRef(obj);
        env->ReleaseStringUTFChars(path, _path);
        return kv;
    }
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_initKv(JNIEnv *env, jobject thiz) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_get(JNIEnv *env, jobject thiz, jstring key, jobject value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    auto cValue = kv->getValue(_key);
    const jclass valueObj = env->GetObjectClass(value);
    const jfieldID typeID = env->GetFieldID(valueObj, "type", "Ljava/lang/String;");
    if (typeID == NULL) {
        return JNI_FALSE;
    }
    /// type int-> 'I', long-> 'L', float-> 'F', double-> 'D', boolean-> 'B', string-> 'S', objectString-> 'OS', None-> 'N'

    if (cValue.isBool()) {
        auto v =  cValue.getBool();
        const jfieldID valueID = env->GetFieldID(valueObj, "boolValue", "Z");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        env->SetBooleanField(value, valueID, v ? JNI_TRUE : JNI_FALSE);
        env->SetObjectField(value,typeID,env->NewStringUTF("B"));
    } else if (cValue.isInt()) {
        /// TODO 需要考虑int是否是int32_t
        const jfieldID valueID = env->GetFieldID(valueObj, "intValue", "I");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        auto v =  cValue.getInt();
        env->SetIntField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("I"));
    } else if (cValue.isLong()) {
        const jfieldID valueID = env->GetFieldID(valueObj, "longValue", "J");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        auto v =  cValue.getLong();
        env->SetLongField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("L"));
    } else if (cValue.isFloat()) {
        const jfieldID valueID = env->GetFieldID(valueObj, "floatValue", "F");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        auto v =  cValue.getFloat();
        env->SetFloatField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("F"));
    } else if (cValue.isDouble()) {
        const jfieldID valueID = env->GetFieldID(valueObj, "doubleValue", "D");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        auto v =  cValue.getDouble();
        env->SetDoubleField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("D"));
    } else if (cValue.isString()) {
        const jfieldID valueID = env->GetFieldID(valueObj, "stringValue", "Ljava/lang/String;");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        string s =  cValue.getString();
        auto v = env->NewStringUTF(s.c_str());
        env->SetObjectField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("S"));
    } else if (cValue.isJsonString()) {
        const jfieldID valueID = env->GetFieldID(valueObj, "stringValue", "Ljava/lang/String;");
        if (valueID == NULL) {
            return JNI_FALSE;
        }
        string s =  cValue.getJsonString();
        auto v = env->NewStringUTF(s.c_str());
        env->SetObjectField(value, valueID, v);
        env->SetObjectField(value,typeID,env->NewStringUTF("OS"));
    } else if (cValue.isNull()) {
        env->SetObjectField(value,typeID,env->NewStringUTF("N"));
    }
    if (env->ExceptionCheck()) {
        return JNI_FALSE;
    }
    env->DeleteLocalRef(valueObj);
    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setBoolean(JNIEnv *env, jobject thiz, jstring key, jboolean value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    kv->saveB(_key, value);
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setInt(JNIEnv *env, jobject thiz, jstring key, jint value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    kv->saveI(_key, int(value));
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setLong(JNIEnv *env, jobject thiz, jstring key, jlong value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    kv->saveL(_key, int(value));
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setFloat(JNIEnv *env, jobject thiz, jstring key, jfloat value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    kv->saveF(_key, float(value));
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setDouble(JNIEnv *env, jobject thiz, jstring key, jdouble value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    kv->saveD(_key, double(value));
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setString(JNIEnv *env, jobject thiz, jstring key, jstring value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    auto _value = jstring2string(env, value);
    kv->saveS(_key, _value, false);
    return  JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_HCache_HCacheAndroid_setObjectString(JNIEnv *env, jobject thiz, jstring key, jstring value) {
    auto kv = getKV(env, thiz);
    if (kv == nullptr) {
        return JNI_FALSE;
    }
    auto _key = jstring2string(env, key);
    auto _value = jstring2string(env, value);
    kv->saveS(_key, _value, true);
    return  JNI_TRUE;
}

};


#endif //HCACHE_CACHEJNI_H