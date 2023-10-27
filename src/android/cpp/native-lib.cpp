#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include "JCOMSIA_HashLib/common.h"
#include "JCOMSIA_HashLib/writeHashLib.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

extern "C" JNIEXPORT jint JNICALL

/**
 * 指定された JPEG ファイルに改ざんチェック値を埋め込んだファイルを出力する。
 * sourceFile と destFile には同じファイルを設定できない。
 */
Java_jp_co_taisei_construction_fieldmanagement_plugin_Camera2Fragment_00024Companion_writeHash(JNIEnv *env, jobject obj, jstring srcFilePath, jstring destFilePath) {

    const char *src = env->GetStringUTFChars(srcFilePath, nullptr);
    const char *dst = env->GetStringUTFChars(destFilePath, nullptr);

    return JCOMSIA_WriteHashValue(src, dst);
}

extern "C" JNIEXPORT jint JNICALL

/**
 * 2つの画像からハッシュ値を計算する
 * @param imagePath 画像改ざん検知情報付与済みの原本画像のパス
 * @param chalkBoardPath 画像改ざん検知情報付与済みの黒板画像のパス
 * @param viewModel hashCodeを持っているオブジェクト
 * @return 終了コード（正常終了またはエラーを示すコード）。処理の途中でエラーにより中断された場合、 hashCode には NULL が格納される。
 */
Java_jp_co_taisei_construction_fieldmanagement_plugin_Camera2Fragment_00024Companion_svgCalculateHash(JNIEnv *env, jobject obj, jstring imagePath, jstring chalkBoardPath, jobject hashCodeObj) {

    const char *image = env->GetStringUTFChars(imagePath, nullptr);
    const char *chalkBoard = env->GetStringUTFChars(chalkBoardPath, nullptr);
    unsigned char *hashCode = NULL;

    int result = JCOMSIA_SVG_CalculateHashValue(image, chalkBoard, &hashCode);

    if (hashCode != NULL) {
        char *hashCodeText = new char[JCOMSIA_HASH_LENGTH];
        memcpy(hashCodeText, hashCode, JCOMSIA_HASH_LENGTH);
        hashCodeText[JCOMSIA_HASH_LENGTH] = '\0';

        jfieldID hashFieldID = env->GetFieldID(env->GetObjectClass(hashCodeObj), "hashCode", "Ljava/lang/String;");
        env->SetObjectField(hashCodeObj, hashFieldID, env->NewStringUTF(hashCodeText));

        delete[] hashCodeText;
    }

    JCOMSIA_SVG_FreeHashValue(&hashCode);

    return result;
}

#pragma clang diagnostic pop
