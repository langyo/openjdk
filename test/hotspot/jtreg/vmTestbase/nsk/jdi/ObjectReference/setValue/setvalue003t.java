/*
 * Copyright (c) 2002, 2025, Oracle and/or its affiliates. All rights reserved.
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

package nsk.jdi.ObjectReference.setValue;

import nsk.share.*;
import nsk.share.jpda.*;
import nsk.share.jdi.*;

/**
 * This is a debuggee class.
 */
public class setvalue003t {
    static Thread testThread = null;

    // tested static fields
    static byte    sByteFld = Byte.MAX_VALUE;
    static short   sShortFld = Short.MAX_VALUE;
    static int     sIntFld = Integer.MAX_VALUE;
    static long    sLongFld = Long.MAX_VALUE;
    static float   sFloatFld = Float.MAX_VALUE;
    static double  sDoubleFld = Double.MAX_VALUE;
    static char    sCharFld = Character.MAX_VALUE;
    static boolean sBooleanFld = true;
    static String  sStrFld = "static field";

    // tested instance fields
    byte    byteFld = sByteFld;
    short   shortFld = sShortFld;
    int     intFld = sIntFld;
    long    longFld = sLongFld;
    float   floatFld = sFloatFld;
    double  doubleFld = sDoubleFld;
    char    charFld = sCharFld;
    boolean booleanFld = false;
    String  strFld = "instance field";

    public static void main(String args[]) {
        System.exit(run(args) + Consts.JCK_STATUS_BASE);
    }

    public static int run(String args[]) {
        return new setvalue003t().runIt(args);
    }

    private int runIt(String args[]) {
        ArgumentHandler argHandler = new ArgumentHandler(args);
        IOPipe setvalue003tPipe = argHandler.createDebugeeIOPipe();

        testThread = Thread.currentThread();
        testThread.setName(setvalue003.DEBUGGEE_THRNAME);

        setvalue003tPipe.println(setvalue003.COMMAND_READY);
        String cmd = setvalue003tPipe.readln();
        if (!cmd.equals(setvalue003.COMMAND_QUIT)) {
            System.err.println("TEST BUG: unknown debugger command: "
                + cmd);
            System.exit(Consts.JCK_STATUS_BASE +
                Consts.TEST_FAILED);
        }
        return Consts.TEST_PASSED;
    }
}
