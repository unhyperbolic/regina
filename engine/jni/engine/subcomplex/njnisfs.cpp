
/**************************************************************************
 *                                                                        *
 *  Regina - A Normal Surface Theory Calculator                           *
 *  Computational Engine                                                  *
 *                                                                        *
 *  Copyright (c) 1999-2002, Ben Burton                                   *
 *  For further details contact Ben Burton (benb@acm.org).                *
 *                                                                        *
 *  This program is free software; you can redistribute it and/or         *
 *  modify it under the terms of the GNU General Public License as        *
 *  published by the Free Software Foundation; either version 2 of the    *
 *  License, or (at your option) any later version.                       *
 *                                                                        *
 *  This program is distributed in the hope that it will be useful, but   *
 *  WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *  General Public License for more details.                              *
 *                                                                        *
 *  You should have received a copy of the GNU General Public             *
 *  License along with this program; if not, write to the Free            *
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,        *
 *  MA 02111-1307, USA.                                                   *
 *                                                                        *
 **************************************************************************/

/* end stub */

#include "subcomplex/nsfs.h"
#include "subcomplex/nlensspace.h"
#include "jnitools.h"
#include "engine/subcomplex/NJNISFS.h"

using namespace regina;

JNIEXPORT jlong JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_getAlpha
        (JNIEnv *env, jobject me, jlong which) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->getFibre(which).alpha;
}

JNIEXPORT jlong JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_getBeta
        (JNIEnv *env, jobject me, jlong which) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->getFibre(which).beta;
}

JNIEXPORT jlong JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_getFibreCount
        (JNIEnv *env, jobject me) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->getFibreCount();
}

JNIEXPORT jlong JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_getOrbitGenus
        (JNIEnv *env, jobject me) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->getOrbitGenus();
}

JNIEXPORT jlong JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_getOrbitPunctures
        (JNIEnv *env, jobject me) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->getOrbitPunctures();
}

JNIEXPORT void JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_insertFibre
        (JNIEnv *env, jobject me, jlong alpha, jlong beta) {
    GET_ENGINE_OBJECT(env, NSFS, me)->insertFibre(
        NExceptionalFibre(alpha, beta));
}

JNIEXPORT jobject JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_isLensSpace
        (JNIEnv *env, jobject me) {
    return CREATE_WRAPPER_OBJECT(env,
        GET_ENGINE_OBJECT(env, NSFS, me)->isLensSpace(),
        "normal/engine/implementation/jni/subcomplex/NJNISFS");
}

JNIEXPORT jboolean JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_isOrbitOrientable
        (JNIEnv *env, jobject me) {
    return GET_ENGINE_OBJECT(env, NSFS, me)->isOrbitOrientable();
}

JNIEXPORT void JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_newNSFS__
        (JNIEnv *env, jobject me) {
    ASSIGN_ENGINE_OBJECT(env, new NSFS(), me);
}

JNIEXPORT void JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_newNSFS__JZJ
        (JNIEnv *env, jobject me, jlong a, jboolean b, jlong c) {
    ASSIGN_ENGINE_OBJECT(env, new NSFS(a, b, c), me);
}

JNIEXPORT void JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_newNSFS__Lnormal_engine_subcomplex_NSFS_2
        (JNIEnv *env, jobject me, jobject you) {
    ASSIGN_ENGINE_OBJECT(env, new NSFS(*GET_ENGINE_OBJECT(env, NSFS, you)), me);
}

JNIEXPORT void JNICALL
        Java_normal_engine_implementation_jni_subcomplex_NJNISFS_reduce
        (JNIEnv *env, jobject me) {
    GET_ENGINE_OBJECT(env, NSFS, me)->reduce();
}

