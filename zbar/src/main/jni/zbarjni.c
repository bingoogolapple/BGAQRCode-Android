/*------------------------------------------------------------------------
 *  Copyright 2010 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/
#include <inttypes.h>
#include <assert.h>
#include <zbar.h>
#include <jni.h>

static jfieldID SymbolSet_peer;
static jfieldID Symbol_peer;
static jfieldID Image_peer, Image_data;
static jfieldID ImageScanner_peer;

static struct {
    int SymbolSet_create, SymbolSet_destroy;
    int Symbol_create, Symbol_destroy;
    int Image_create, Image_destroy;
    int ImageScanner_create, ImageScanner_destroy;
} stats;


#define PEER_CAST(l) \
    ((void*)(uintptr_t)(l))

#define GET_PEER(c, o) \
    PEER_CAST((*env)->GetLongField(env, (o), c ## _peer))


static inline void
throw_exc(JNIEnv *env,
          const char *name,
          const char *msg)
{
    jclass cls = (*env)->FindClass(env, name);
    if(cls)
        (*env)->ThrowNew(env, cls, msg);
    (*env)->DeleteLocalRef(env, cls);
}

static inline uint32_t
format_to_fourcc(JNIEnv *env,
                 jstring format)
{
    if(!format)
        goto invalid;

    int n = (*env)->GetStringLength(env, format);
    if(0 >= n || n > 4)
        goto invalid;

    char fmtstr[8];
    (*env)->GetStringUTFRegion(env, format, 0, n, fmtstr);

    uint32_t fourcc = 0;
    int i;
    for(i = 0; i < n; i++) {
        if(fmtstr[i] < ' ' || 'Z' < fmtstr[i] ||
           ('9' < fmtstr[i] && fmtstr[i] < 'A') ||
           (' ' < fmtstr[i] && fmtstr[i] < '0'))
            goto invalid;
        fourcc |= ((uint32_t)fmtstr[i]) << (8 * i);
    }
    return(fourcc);

invalid:
    throw_exc(env, "java/lang/IllegalArgumentException",
              "invalid format fourcc");
    return(0);
}

static JavaVM *jvm = NULL;

JNIEXPORT jint JNICALL
JNI_OnLoad (JavaVM *_jvm,
            void *reserved)
{
    jvm = _jvm;
    return(JNI_VERSION_1_2);
}

JNIEXPORT void JNICALL
JNI_OnUnload (JavaVM *_jvm,
              void *reserved)
{
    assert(stats.SymbolSet_create == stats.SymbolSet_destroy);
    assert(stats.Symbol_create == stats.Symbol_destroy);
    assert(stats.Image_create == stats.Image_destroy);
    assert(stats.ImageScanner_create == stats.ImageScanner_destroy);
}


JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_SymbolSet_init (JNIEnv *env,
                                          jclass cls)
{
    SymbolSet_peer = (*env)->GetFieldID(env, cls, "peer", "J");
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_SymbolSet_destroy (JNIEnv *env,
                                             jobject obj,
                                             jlong peer)
{
    zbar_symbol_set_ref(PEER_CAST(peer), -1);
    stats.SymbolSet_destroy++;
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_SymbolSet_size (JNIEnv *env,
                                          jobject obj)
{
    zbar_symbol_set_t *zsyms = GET_PEER(SymbolSet, obj);
    if(!zsyms)
        return(0);
    return(zbar_symbol_set_get_size(zsyms));
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_SymbolSet_firstSymbol (JNIEnv *env,
                                                 jobject obj,
                                                 jlong peer)
{
    if(!peer)
        return(0);
    const zbar_symbol_t *zsym = zbar_symbol_set_first_symbol(PEER_CAST(peer));
    if(zsym) {
        zbar_symbol_ref(zsym, 1);
        stats.Symbol_create++;
    }
    return((intptr_t)zsym);
}



JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Symbol_init (JNIEnv *env,
                                       jclass cls)
{
    Symbol_peer = (*env)->GetFieldID(env, cls, "peer", "J");
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Symbol_destroy (JNIEnv *env,
                                          jobject obj,
                                          jlong peer)
{
    zbar_symbol_ref(PEER_CAST(peer), -1);
    stats.Symbol_destroy++;
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getType (JNIEnv *env,
                                          jobject obj,
                                          jlong peer)
{
    return(zbar_symbol_get_type(PEER_CAST(peer)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getConfigMask (JNIEnv *env,
                                                jobject obj)
{
    return(zbar_symbol_get_configs(GET_PEER(Symbol, obj)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getModifierMask (JNIEnv *env,
                                                  jobject obj)
{
    return(zbar_symbol_get_modifiers(GET_PEER(Symbol, obj)));
}

JNIEXPORT jstring JNICALL
Java_net_sourceforge_zbar_Symbol_getData (JNIEnv *env,
                                          jobject obj)
{
    const char *data = zbar_symbol_get_data(GET_PEER(Symbol, obj));
    return((*env)->NewStringUTF(env, data));
}

JNIEXPORT jstring JNICALL
Java_net_sourceforge_zbar_Symbol_getDataBytes (JNIEnv *env,
                                               jobject obj)
{
    const zbar_symbol_t *zsym = GET_PEER(Symbol, obj);
    const void *data = zbar_symbol_get_data(zsym);
    unsigned long datalen = zbar_symbol_get_data_length(zsym);
    if(!data || !datalen)
        return(NULL);

    jbyteArray bytes = (*env)->NewByteArray(env, datalen);
    if(!bytes)
        return(NULL);

    (*env)->SetByteArrayRegion(env, bytes, 0, datalen, data);
    return(bytes);
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getQuality (JNIEnv *env,
                                             jobject obj)
{
    return(zbar_symbol_get_quality(GET_PEER(Symbol, obj)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getCount (JNIEnv *env,
                                           jobject obj)
{
    return(zbar_symbol_get_count(GET_PEER(Symbol, obj)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getLocationSize (JNIEnv *env,
                                                  jobject obj,
                                                  jlong peer)
{
    return(zbar_symbol_get_loc_size(PEER_CAST(peer)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getLocationX (JNIEnv *env,
                                               jobject obj,
                                               jlong peer,
                                               jint idx)
{
    return(zbar_symbol_get_loc_x(PEER_CAST(peer), idx));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getLocationY (JNIEnv *env,
                                               jobject obj,
                                               jlong peer,
                                               jint idx)
{
    return(zbar_symbol_get_loc_y(PEER_CAST(peer), idx));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Symbol_getOrientation (JNIEnv *env,
                                                 jobject obj)
{
    return(zbar_symbol_get_orientation(GET_PEER(Symbol, obj)));
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_Symbol_getComponents (JNIEnv *env,
                                                jobject obj,
                                                jlong peer)
{
    const zbar_symbol_set_t *zsyms =
        zbar_symbol_get_components(PEER_CAST(peer));
    if(zsyms) {
        zbar_symbol_set_ref(zsyms, 1);
        stats.SymbolSet_create++;
    }
    return((intptr_t)zsyms);
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_Symbol_next (JNIEnv *env,
                                       jobject obj)
{
    const zbar_symbol_t *zsym = zbar_symbol_next(GET_PEER(Symbol, obj));
    if(zsym) {
        zbar_symbol_ref(zsym, 1);
        stats.Symbol_create++;
    }
    return((intptr_t)zsym);
}



static void
Image_cleanupByteArray (zbar_image_t *zimg)
{
    jobject data = zbar_image_get_userdata(zimg);
    assert(data);

    JNIEnv *env = NULL;
    if((*jvm)->AttachCurrentThread(jvm, (void*)&env, NULL))
        return;
    assert(env);
    if(env && data) {
        void *raw = (void*)zbar_image_get_data(zimg);
        assert(raw);
        /* const image data is unchanged - abort copy back */
        (*env)->ReleaseByteArrayElements(env, data, raw, JNI_ABORT);
        (*env)->DeleteGlobalRef(env, data);
        zbar_image_set_userdata(zimg, NULL);
    }
}

static void
Image_cleanupIntArray (zbar_image_t *zimg)
{
    jobject data = zbar_image_get_userdata(zimg);
    assert(data);

    JNIEnv *env = NULL;
    if((*jvm)->AttachCurrentThread(jvm, (void*)&env, NULL))
        return;
    assert(env);
    if(env && data) {
        void *raw = (void*)zbar_image_get_data(zimg);
        assert(raw);
        /* const image data is unchanged - abort copy back */
        (*env)->ReleaseIntArrayElements(env, data, raw, JNI_ABORT);
        (*env)->DeleteGlobalRef(env, data);
        zbar_image_set_userdata(zimg, NULL);
    }
}


JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_init (JNIEnv *env,
                                      jclass cls)
{
    Image_peer = (*env)->GetFieldID(env, cls, "peer", "J");
    Image_data = (*env)->GetFieldID(env, cls, "data", "Ljava/lang/Object;");
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_Image_create (JNIEnv *env,
                                        jobject obj)
{
    zbar_image_t *zimg = zbar_image_create();
    if(!zimg) {
        throw_exc(env, "java/lang/OutOfMemoryError", NULL);
        return(0);
    }
    stats.Image_create++;
    return((intptr_t)zimg);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_destroy (JNIEnv *env,
                                         jobject obj,
                                         jlong peer)
{
    zbar_image_ref(PEER_CAST(peer), -1);
    stats.Image_destroy++;
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_Image_convert (JNIEnv *env,
                                         jobject obj,
                                         jlong peer,
                                         jstring format)
{
    uint32_t fourcc = format_to_fourcc(env, format);
    if(!fourcc)
        return(0);
    zbar_image_t *zimg = zbar_image_convert(PEER_CAST(peer), fourcc);
    if(!zimg)
        throw_exc(env, "java/lang/UnsupportedOperationException",
                  "unsupported image format");
    else
        stats.Image_create++;
    return((intptr_t)zimg);
}

JNIEXPORT jstring JNICALL
Java_net_sourceforge_zbar_Image_getFormat (JNIEnv *env,
                                           jobject obj)
{
    uint32_t fourcc = zbar_image_get_format(GET_PEER(Image, obj));
    if(!fourcc)
        return(NULL);
    char fmtstr[5] = { fourcc, fourcc >> 8, fourcc >> 16, fourcc >> 24, 0 };
    return((*env)->NewStringUTF(env, fmtstr));
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setFormat (JNIEnv *env,
                                           jobject obj,
                                           jstring format)
{
    uint32_t fourcc = format_to_fourcc(env, format);
    if(!fourcc)
        return;
    zbar_image_set_format(GET_PEER(Image, obj), fourcc);
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Image_getSequence (JNIEnv *env,
                                             jobject obj)
{
    return(zbar_image_get_sequence(GET_PEER(Image, obj)));
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setSequence (JNIEnv *env,
                                             jobject obj,
                                             jint seq)
{
    zbar_image_set_sequence(GET_PEER(Image, obj), seq);
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Image_getWidth (JNIEnv *env,
                                          jobject obj)
{
    return(zbar_image_get_width(GET_PEER(Image, obj)));
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_Image_getHeight (JNIEnv *env,
                                           jobject obj)
{
    return(zbar_image_get_height(GET_PEER(Image, obj)));
}

JNIEXPORT jobject JNICALL
Java_net_sourceforge_zbar_Image_getSize (JNIEnv *env,
                                         jobject obj)
{
    jintArray size = (*env)->NewIntArray(env, 2);
    if(!size)
        return(NULL);

    unsigned dims[2];
    zbar_image_get_size(GET_PEER(Image, obj), dims, dims + 1);
    jint jdims[2] = { dims[0], dims[1] };
    (*env)->SetIntArrayRegion(env, size, 0, 2, jdims);
    return(size);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setSize__II (JNIEnv *env,
                                             jobject obj,
                                             jint width,
                                             jint height)
{
    if(width < 0) width = 0;
    if(height < 0) height = 0;
    zbar_image_set_size(GET_PEER(Image, obj), width, height);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setSize___3I (JNIEnv *env,
                                              jobject obj,
                                              jintArray size)
{
    if((*env)->GetArrayLength(env, size) != 2)
        throw_exc(env, "java/lang/IllegalArgumentException",
                  "size must be an array of two ints");
    jint dims[2];
    (*env)->GetIntArrayRegion(env, size, 0, 2, dims);
    if(dims[0] < 0) dims[0] = 0;
    if(dims[1] < 0) dims[1] = 0;
    zbar_image_set_size(GET_PEER(Image, obj), dims[0], dims[1]);
}

JNIEXPORT jobject JNICALL
Java_net_sourceforge_zbar_Image_getCrop (JNIEnv *env,
                                         jobject obj)
{
    jintArray crop = (*env)->NewIntArray(env, 4);
    if(!crop)
        return(NULL);

    unsigned dims[4];
    zbar_image_get_crop(GET_PEER(Image, obj), dims, dims + 1,
                        dims + 2, dims + 3);
    jint jdims[4] = { dims[0], dims[1], dims[2], dims[3] };
    (*env)->SetIntArrayRegion(env, crop, 0, 4, jdims);
    return(crop);
}

#define VALIDATE_CROP(u, m) \
    if((u) < 0) {           \
        (m) += (u);         \
        (u) = 0;            \
    }

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setCrop__IIII (JNIEnv *env,
                                               jobject obj,
                                               jint x, jint y,
                                               jint w, jint h)
{
    VALIDATE_CROP(x, w);
    VALIDATE_CROP(y, h);
    zbar_image_set_crop(GET_PEER(Image, obj), x, y, w, h);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setCrop___3I (JNIEnv *env,
                                              jobject obj,
                                              jintArray crop)
{
    if((*env)->GetArrayLength(env, crop) != 4)
        throw_exc(env, "java/lang/IllegalArgumentException",
                  "crop must be an array of four ints");
    jint dims[4];
    (*env)->GetIntArrayRegion(env, crop, 0, 4, dims);
    VALIDATE_CROP(dims[0], dims[2]);
    VALIDATE_CROP(dims[1], dims[3]);
    zbar_image_set_crop(GET_PEER(Image, obj),
                        dims[0], dims[1], dims[2], dims[3]);
}
#undef VALIDATE_CROP

JNIEXPORT jobject JNICALL
Java_net_sourceforge_zbar_Image_getData (JNIEnv *env,
                                         jobject obj)
{
    jobject data = (*env)->GetObjectField(env, obj, Image_data);
    if(data)
        return(data);

    zbar_image_t *zimg = GET_PEER(Image, obj);
    data = zbar_image_get_userdata(zimg);
    if(data)
        return(data);

    unsigned long rawlen = zbar_image_get_data_length(zimg);
    const void *raw = zbar_image_get_data(zimg);
    if(!rawlen || !raw)
        return(NULL);

    data = (*env)->NewByteArray(env, rawlen);
    if(!data)
        return(NULL);

    (*env)->SetByteArrayRegion(env, data, 0, rawlen, raw);
    (*env)->SetObjectField(env, obj, Image_data, data);
    return(data);
}

static inline void
Image_setData (JNIEnv *env,
               jobject obj,
               jbyteArray data,
               void *raw,
               unsigned long rawlen,
               zbar_image_cleanup_handler_t *cleanup)
{
    if(!data)
        cleanup = NULL;
    (*env)->SetObjectField(env, obj, Image_data, data);
    zbar_image_t *zimg = GET_PEER(Image, obj);
    zbar_image_set_data(zimg, raw, rawlen, cleanup);
    zbar_image_set_userdata(zimg, (*env)->NewGlobalRef(env, data));
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setData___3B (JNIEnv *env,
                                              jobject obj,
                                              jbyteArray data)
{
    jbyte *raw = NULL;
    unsigned long rawlen = 0;
    if(data) {
        raw = (*env)->GetByteArrayElements(env, data, NULL);
        if(!raw)
            return;
        rawlen = (*env)->GetArrayLength(env, data);
    }
    Image_setData(env, obj, data, raw, rawlen, Image_cleanupByteArray);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_Image_setData___3I (JNIEnv *env,
                                              jobject obj,
                                              jintArray data)
{
    jint *raw = NULL;
    unsigned long rawlen = 0;
    if(data) {
        raw = (*env)->GetIntArrayElements(env, data, NULL);
        if(!raw)
            return;
        rawlen = (*env)->GetArrayLength(env, data) * sizeof(*raw);
    }
    Image_setData(env, obj, data, raw, rawlen, Image_cleanupIntArray);
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_Image_getSymbols (JNIEnv *env,
                                            jobject obj,
                                            jlong peer)
{
    const zbar_symbol_set_t *zsyms = zbar_image_get_symbols(PEER_CAST(peer));
    if(zsyms) {
        zbar_symbol_set_ref(zsyms, 1);
        stats.SymbolSet_create++;
    }
    return((intptr_t)zsyms);
}


JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_ImageScanner_init (JNIEnv *env,
                                             jclass cls)
{
    ImageScanner_peer = (*env)->GetFieldID(env, cls, "peer", "J");
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_ImageScanner_create (JNIEnv *env,
                                               jobject obj)
{
    zbar_image_scanner_t *zscn = zbar_image_scanner_create();
    if(!zscn) {
        throw_exc(env, "java/lang/OutOfMemoryError", NULL);
        return(0);
    }
    stats.ImageScanner_create++;
    return((intptr_t)zscn);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_ImageScanner_destroy (JNIEnv *env,
                                                jobject obj,
                                                jlong peer)
{
    zbar_image_scanner_destroy(PEER_CAST(peer));
    stats.ImageScanner_destroy++;
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_ImageScanner_setConfig (JNIEnv *env,
                                                  jobject obj,
                                                  jint symbology,
                                                  jint config,
                                                  jint value)
{
    zbar_image_scanner_set_config(GET_PEER(ImageScanner, obj),
                                  symbology, config, value);
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_ImageScanner_parseConfig (JNIEnv *env,
                                                    jobject obj,
                                                    jstring cfg)
{
    const char *cfgstr = (*env)->GetStringUTFChars(env, cfg, NULL);
    if(!cfgstr)
        return;
    if(zbar_image_scanner_parse_config(GET_PEER(ImageScanner, obj), cfgstr))
        throw_exc(env, "java/lang/IllegalArgumentException",
                  "unknown configuration");
}

JNIEXPORT void JNICALL
Java_net_sourceforge_zbar_ImageScanner_enableCache (JNIEnv *env,
                                                    jobject obj,
                                                    jboolean enable)
{
    zbar_image_scanner_enable_cache(GET_PEER(ImageScanner, obj), enable);
}

JNIEXPORT jlong JNICALL
Java_net_sourceforge_zbar_ImageScanner_getResults (JNIEnv *env,
                                                   jobject obj,
                                                   jlong peer)
{
    const zbar_symbol_set_t *zsyms =
        zbar_image_scanner_get_results(PEER_CAST(peer));
    if(zsyms) {
        zbar_symbol_set_ref(zsyms, 1);
        stats.SymbolSet_create++;
    }
    return((intptr_t)zsyms);
}

JNIEXPORT jint JNICALL
Java_net_sourceforge_zbar_ImageScanner_scanImage (JNIEnv *env,
                                                  jobject obj,
                                                  jobject image)
{
    zbar_image_scanner_t *zscn = GET_PEER(ImageScanner, obj);
    zbar_image_t *zimg = GET_PEER(Image, image);

    int n = zbar_scan_image(zscn, zimg);
    if(n < 0)
        throw_exc(env, "java/lang/UnsupportedOperationException",
                  "unsupported image format");
    return(n);
}
