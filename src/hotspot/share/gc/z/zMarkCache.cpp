/*
 * Copyright (c) 2016, 2025, Oracle and/or its affiliates. All rights reserved.
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
 */

#include "gc/z/zMarkCache.inline.hpp"
#include "utilities/globalDefinitions.hpp"
#include "utilities/powerOfTwo.hpp"

static size_t shift_for_stripes(size_t nstripes) {
  return ZMarkStripeShift + (size_t)log2i_exact(nstripes);
}

ZMarkCacheEntry::ZMarkCacheEntry()
  : _page(nullptr),
    _objects(0),
    _bytes(0) {}

ZMarkCache::ZMarkCache(size_t nstripes)
  : _shift(shift_for_stripes(nstripes)) {}

ZMarkCache::~ZMarkCache() {
  // Evict all entries
  for (size_t i = 0; i < ZMarkCacheSize; i++) {
    _cache[i].evict();
  }
}

void ZMarkCache::set_nstripes(size_t nstripes) {
  _shift = shift_for_stripes(nstripes);
}
