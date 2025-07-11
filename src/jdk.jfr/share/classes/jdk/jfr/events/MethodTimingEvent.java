/*
 * Copyright (c) 2025, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
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
package jdk.jfr.events;

import jdk.jfr.Category;
import jdk.jfr.Label;
import jdk.jfr.Name;
import jdk.jfr.Timespan;
import jdk.jfr.internal.RemoveFields;
import jdk.jfr.Description;

@Name("jdk.MethodTiming")
@Label("Method Timing")
@Category({ "Java Virtual Machine", "Method Tracing" })
@RemoveFields({ "duration", "eventThread", "stackTrace" })
@Description("Measures the approximate time it takes for a method to execute, including all delays, not just CPU processing time")
public final class MethodTimingEvent extends AbstractJDKEvent {

    @Label("Method")
    public long method;

    @Label("Invocations")
    public long invocations;

    @Label("Minimum Time")
    @Description("The value may be missing (Long.MIN_VALUE) if the clock-resolution is too low to establish a minimum time")
    @Timespan(Timespan.TICKS)
    public long minimum;

    @Label("Average Time")
    @Description("The value may be missing (Long.MIN_VALUE) if the clock-resolution is too low to establish an average time")
    @Timespan(Timespan.TICKS)
    public long average;

    @Label("Maximum Time")
    @Description("The value may be missing (Long.MIN_VALUE) if the clock-resolution is too low to establish a maximum time")
    @Timespan(Timespan.TICKS)
    public long maximum;

    public static void commit(long start, long method, long invocations, long minimum, long average, long maximum) {
        // Generated by JFR
    }

    public static long timestamp() {
        // Generated by JFR
        return 0;
    }

    public static boolean enabled() {
        // Generated by JFR
        return false;
    }
}
