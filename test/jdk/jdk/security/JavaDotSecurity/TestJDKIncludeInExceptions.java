/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
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

import java.security.Security;

/**
 * @test
 * @bug 8207846 8208691
 * @summary Test the default setting of the jdk.net.includeInExceptions
 *          security property
 * @comment In OpenJDK, this property has value "hostInfoExclSocket" by default
 *          This test assures the default is not changed.
 * @run main TestJDKIncludeInExceptions
 */
public class TestJDKIncludeInExceptions {

    public static void main(String args[]) throws Exception {
        String incInExc = Security.getProperty("jdk.includeInExceptions");
        if (incInExc == null || !incInExc.equals("hostInfoExclSocket")) {
            throw new RuntimeException("Test failed: default value of " +
                "jdk.includeInExceptions security property does not have expected value: " +
                incInExc);
        }
    }
}
