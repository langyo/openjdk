/*
 * Copyright (c) 2021, 2024, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_OOPS_ARRAY_INLINE_HPP
#define SHARE_OOPS_ARRAY_INLINE_HPP

#include "oops/array.hpp"

#include "memory/allocation.hpp"
#include "memory/metaspace.hpp"

template <typename T>
inline void* Array<T>::operator new(size_t size, ClassLoaderData* loader_data, int length, TRAPS) throw() {
  size_t word_size = Array::size(length);
  return (void*) Metaspace::allocate(loader_data, word_size,
                                     MetaspaceObj::array_type(sizeof(T)), false, THREAD);
}

template <typename T>
inline void* Array<T>::operator new(size_t size, ClassLoaderData* loader_data, int length) throw() {
  size_t word_size = Array::size(length);
  return (void*) Metaspace::allocate(loader_data, word_size,
                                     MetaspaceObj::array_type(sizeof(T)), false);
}

template <typename T>
inline void* Array<T>::operator new(size_t size, int length, MemTag flags) throw() {
  size = Array::size(length) * BytesPerWord;
  void* p = AllocateHeap(size * BytesPerWord, flags);
  memset(p, 0, size);
  return p;
}

#endif // SHARE_OOPS_ARRAY_INLINE_HPP
