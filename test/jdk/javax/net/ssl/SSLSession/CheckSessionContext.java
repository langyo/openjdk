/*
 * Copyright (c) 2020, 2024, Oracle and/or its affiliates. All rights reserved.
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

/*
 * @test
 * @library /javax/net/ssl/templates
 * @bug 8242008
 * @summary Verify SSLSession.getSessionContext() is not null for the initial
 *          and the resumed session
 * @run main/othervm -Djdk.tls.client.protocols=TLSv1.2 -Djdk.tls.server.enableSessionTicketExtension=true -Djdk.tls.client.enableSessionTicketExtension=false CheckSessionContext
 * @run main/othervm -Djdk.tls.client.protocols=TLSv1.2 -Djdk.tls.server.enableSessionTicketExtension=true -Djdk.tls.client.enableSessionTicketExtension=true CheckSessionContext
 * @run main/othervm -Djdk.tls.client.protocols=TLSv1.3 -Djdk.tls.server.enableSessionTicketExtension=true -Djdk.tls.client.enableSessionTicketExtension=false CheckSessionContext
 * @run main/othervm -Djdk.tls.client.protocols=TLSv1.3 -Djdk.tls.server.enableSessionTicketExtension=true -Djdk.tls.client.enableSessionTicketExtension=true CheckSessionContext
 * @run main/othervm -Djdk.tls.client.protocols=TLSv1.3 -Djdk.tls.server.enableSessionTicketExtension=false -Djdk.tls.client.enableSessionTicketExtension=true CheckSessionContext
 *
 */

import javax.net.ssl.SSLSession;
import java.util.HexFormat;

public class CheckSessionContext {

    static HexFormat hex = HexFormat.of();

    public static void main(String[] args) throws Exception {
        TLSBase.Server server = new TLSBase.Server();

        // Initial client session
        TLSBase.Client client1 = new TLSBase.Client();
        client1.connect();
        if (server.getSession(client1).getSessionContext() == null) {
            throw new Exception("Context was null.  Handshake failure.");
        } else {
            System.out.println("Context was found");
        }
        SSLSession ss = server.getSession(client1);
        System.out.println(ss);
        byte[] id = ss.getId();
        System.out.println("id = " + hex.formatHex(id));
        System.out.println("ss.getSessionContext().getSession(id) = " + ss.getSessionContext().getSession(id));
        if (ss.getSessionContext().getSession(id) != null) {
            id = ss.getSessionContext().getSession(id).getId();
            System.out.println("id = " + hex.formatHex(id));
        }
        server.close(client1);
        client1.close();

        // Resume the client session
        TLSBase.Client client2 = new TLSBase.Client();
        client2.connect();
        if (server.getSession(client2).getSessionContext() == null) {
            throw new Exception("Context was null on resumption");
        } else {
            System.out.println("Context was found on resumption");
        }
        server.close(client2);
        client2.close();
    }
}
