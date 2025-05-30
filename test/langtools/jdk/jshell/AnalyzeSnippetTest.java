/*
 * Copyright (c) 2017, 2024, Oracle and/or its affiliates. All rights reserved.
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
 * @bug 8182270 8341176
 * @summary test non-eval Snippet analysis
 * @build KullaTesting TestingInputStream
 * @run testng AnalyzeSnippetTest
 */

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.List;
import java.util.stream.Stream;
import jdk.jshell.Snippet;
import jdk.jshell.DeclarationSnippet;
import jdk.jshell.Diag;
import org.testng.annotations.Test;

import jdk.jshell.JShell;
import jdk.jshell.MethodSnippet;
import static org.testng.Assert.assertFalse;
import static org.testng.Assert.assertTrue;
import static org.testng.Assert.assertEquals;
import org.testng.annotations.AfterMethod;
import org.testng.annotations.BeforeMethod;
import jdk.jshell.ErroneousSnippet;
import jdk.jshell.ExpressionSnippet;
import jdk.jshell.ImportSnippet;
import jdk.jshell.Snippet.SubKind;
import jdk.jshell.SourceCodeAnalysis;
import jdk.jshell.StatementSnippet;
import jdk.jshell.TypeDeclSnippet;
import jdk.jshell.VarSnippet;
import static jdk.jshell.Snippet.SubKind.*;
import jdk.jshell.SourceCodeAnalysis.SnippetWrapper;

@Test
public class AnalyzeSnippetTest {

    JShell state;
    SourceCodeAnalysis sca;

    @BeforeMethod
    public void setUp() {
        state = JShell.builder()
                .out(new PrintStream(new ByteArrayOutputStream()))
                .err(new PrintStream(new ByteArrayOutputStream()))
                .executionEngine(Presets.TEST_DEFAULT_EXECUTION)
                .build();
        sca = state.sourceCodeAnalysis();
    }

    @AfterMethod
    public void tearDown() {
        if (state != null) {
            state.close();
        }
        state = null;
        sca = null;
    }

    public void testImport() {
        ImportSnippet sn = (ImportSnippet) assertSnippet("import java.util.List;",
                SubKind.SINGLE_TYPE_IMPORT_SUBKIND);
        assertEquals(sn.name(), "List");
        sn = (ImportSnippet) assertSnippet("import static java.nio.file.StandardOpenOption.CREATE;",
                SubKind.SINGLE_STATIC_IMPORT_SUBKIND);
        assertTrue(sn.isStatic());
    }

    public void testClass() {
        TypeDeclSnippet sn = (TypeDeclSnippet) assertSnippet("class C {}",
                SubKind.CLASS_SUBKIND);
        assertEquals(sn.name(), "C");
        sn = (TypeDeclSnippet) assertSnippet("enum EE {A, B , C}",
                SubKind.ENUM_SUBKIND);
    }

    public void testMethod() {
        MethodSnippet sn = (MethodSnippet) assertSnippet("int m(int x) { return x + x; }",
                SubKind.METHOD_SUBKIND);
        assertEquals(sn.name(), "m");
        assertEquals(sn.signature(), "(int)int");
    }

    public void testVar() {
        VarSnippet sn = (VarSnippet) assertSnippet("int i;",
                SubKind.VAR_DECLARATION_SUBKIND);
        assertEquals(sn.name(), "i");
        assertEquals(sn.typeName(), "int");
        sn = (VarSnippet) assertSnippet("int jj = 6;",
                SubKind.VAR_DECLARATION_WITH_INITIALIZER_SUBKIND);
        sn = (VarSnippet) assertSnippet("2 + 2",
                SubKind.TEMP_VAR_EXPRESSION_SUBKIND);
    }

    public void testExpression() {
        state.eval("int aa = 10;");
        ExpressionSnippet sn = (ExpressionSnippet) assertSnippet("aa",
                SubKind.VAR_VALUE_SUBKIND);
        assertEquals(sn.name(), "aa");
        assertEquals(sn.typeName(), "int");
        sn = (ExpressionSnippet) assertSnippet("aa;",
                SubKind.VAR_VALUE_SUBKIND);
        assertEquals(sn.name(), "aa");
        assertEquals(sn.typeName(), "int");
        sn = (ExpressionSnippet) assertSnippet("aa = 99",
                SubKind.ASSIGNMENT_SUBKIND);
    }

    public void testStatement() {
        StatementSnippet sn = (StatementSnippet) assertSnippet("System.out.println(33)",
                SubKind.STATEMENT_SUBKIND);
        sn = (StatementSnippet) assertSnippet("if (true) System.out.println(33);",
                SubKind.STATEMENT_SUBKIND);
    }

    public void testErroneous() {
        ErroneousSnippet sn = (ErroneousSnippet) assertSnippet("+++",
                SubKind.UNKNOWN_SUBKIND);
        sn = (ErroneousSnippet) assertSnippet("abc",
                SubKind.UNKNOWN_SUBKIND);
    }

    public void testDiagnosticsForSourceSnippet() {
        Snippet sn;
        sn = assertSnippet("unknown()", UNKNOWN_SUBKIND);
        assertDiagnostics(sn, "0-7:compiler.err.cant.resolve.location.args");
        sn = assertSnippet("new String(null, )", UNKNOWN_SUBKIND);
        assertDiagnostics(sn, "17-17:compiler.err.illegal.start.of.expr");
        sn = assertSnippet("1 + ", UNKNOWN_SUBKIND);
        assertDiagnostics(sn, "3-3:compiler.err.premature.eof");
        sn = assertSnippet("class C {", UNKNOWN_SUBKIND);
        assertDiagnostics(sn, "9-9:compiler.err.premature.eof");
        sn = assertSnippet("class C {}", CLASS_SUBKIND);
        assertDiagnostics(sn);
        sn = assertSnippet("void t() { throw new java.io.IOException(); }", METHOD_SUBKIND);
        assertDiagnostics(sn, "11-43:compiler.err.unreported.exception.need.to.catch.or.throw");
        sn = assertSnippet("void t() { unknown(); }", METHOD_SUBKIND);
        assertDiagnostics(sn, "11-18:compiler.err.cant.resolve.location.args");
        sn = assertSnippet("import unknown.unknown;", SINGLE_TYPE_IMPORT_SUBKIND);
        assertDiagnostics(sn, "7-22:compiler.err.doesnt.exist");
    }

    public void testSnippetWrapper() {
        SourceCodeAnalysis analysis = state.sourceCodeAnalysis();
        Snippet sn;
        String code = "unknown()";
        sn = assertSnippet(code, UNKNOWN_SUBKIND);
        SnippetWrapper wrapper = analysis.wrapper(sn);
        String wrapped = wrapper.wrapped();
        assertEquals(wrapped, """
                              package REPL;

                              class $JShell$DOESNOTMATTER {
                                  public static java.lang.Object do_it$() throws java.lang.Throwable {
                                      return unknown();
                                  }
                              }
                              """);
        for (int pos = 0; pos < code.length(); pos++) {
            int wrappedPos = wrapper.sourceToWrappedPosition(pos);
            assertEquals(wrapped.charAt(wrappedPos), code.charAt(pos));
            assertEquals(wrapper.wrappedToSourcePosition(wrappedPos), pos);
        }
    }

    public void testNoStateChange() {
        assertSnippet("int a = 5;", SubKind.VAR_DECLARATION_WITH_INITIALIZER_SUBKIND);
        assertSnippet("a", SubKind.UNKNOWN_SUBKIND);
        VarSnippet vsn = (VarSnippet) state.eval("int aa = 10;").get(0).snippet();
        assertSnippet("++aa;", SubKind.TEMP_VAR_EXPRESSION_SUBKIND);
        assertEquals(state.varValue(vsn), "10");
        assertSnippet("class CC {}", SubKind.CLASS_SUBKIND);
        assertSnippet("new CC();", SubKind.UNKNOWN_SUBKIND);
    }

    private Snippet assertSnippet(String input, SubKind sk) {
        List<Snippet> sns = sca.sourceToSnippets(input);
        assertEquals(sns.size(), 1, "snippet count");
        Snippet sn = sns.get(0);
        assertEquals(sn.id(), "*UNASSOCIATED*");
        assertEquals(sn.subKind(), sk);
        return sn;
    }

    private String diagToString(Diag d) {
        return d.getStartPosition() + "-" + d.getEndPosition() + ":" + d.getCode();
    }

    private void assertDiagnostics(Snippet s, String... expectedDiags) {
        List<String> actual = state.diagnostics(s).map(this::diagToString).toList();
        List<String> expected = List.of(expectedDiags);
        assertEquals(actual, expected);
    }
}
