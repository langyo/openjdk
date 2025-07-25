/*
 * Copyright (c) 1997, 2025, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_RUNTIME_SHAREDRUNTIME_HPP
#define SHARE_RUNTIME_SHAREDRUNTIME_HPP

#include "classfile/compactHashtable.hpp"
#include "code/codeBlob.hpp"
#include "code/vmreg.hpp"
#include "interpreter/linkResolver.hpp"
#include "memory/allStatic.hpp"
#include "memory/metaspaceClosure.hpp"
#include "memory/resourceArea.hpp"
#include "runtime/stubInfo.hpp"
#include "utilities/macros.hpp"

class AdapterHandlerEntry;
class AdapterFingerPrint;
class vframeStream;

// Runtime is the base class for various runtime interfaces
// (InterpreterRuntime, CompilerRuntime, etc.). It provides
// shared functionality such as exception forwarding (C++ to
// Java exceptions), locking/unlocking mechanisms, statistical
// information, etc.

class SharedRuntime: AllStatic {
 private:
  // Declare shared stub fields
#define SHARED_STUB_FIELD_DECLARE(name, type) \
  static type*       BLOB_FIELD_NAME(name);
  SHARED_STUBS_DO(SHARED_STUB_FIELD_DECLARE)
#undef SHARED_STUB_FIELD_DECLARE

#ifdef ASSERT
  static bool is_resolve_id(StubId id) {
    return (id == StubId::shared_wrong_method_id ||
            id == StubId::shared_wrong_method_abstract_id ||
            id == StubId::shared_ic_miss_id ||
            id == StubId::shared_resolve_opt_virtual_call_id ||
            id == StubId::shared_resolve_virtual_call_id ||
            id == StubId::shared_resolve_static_call_id);
  }
  static bool is_polling_page_id(StubId id) {
    return (id == StubId::shared_polling_page_vectors_safepoint_handler_id ||
            id == StubId::shared_polling_page_safepoint_handler_id ||
            id == StubId::shared_polling_page_return_handler_id);
  }
  static bool is_throw_id(StubId id) {
    return (id == StubId::shared_throw_AbstractMethodError_id ||
            id == StubId::shared_throw_IncompatibleClassChangeError_id ||
            id == StubId::shared_throw_NullPointerException_at_call_id ||
            id == StubId::shared_throw_StackOverflowError_id ||
            id == StubId::shared_throw_delayed_StackOverflowError_id);
  }
#endif

  // cont_doYieldStub is not yet folded into the general model for
  // shared stub/blob handling. It is actually a specially generated
  // native wrapper for a specific native method, as also is it's
  // counterpart the continuation do_enter method.
  static nmethod*            _cont_doYield_stub;

#ifndef PRODUCT
  // Counters
  static int64_t _nof_megamorphic_calls;         // total # of megamorphic calls (through vtable)
#endif // !PRODUCT

 private:
  static SafepointBlob* generate_handler_blob(StubId id, address call_ptr);
  static RuntimeStub*   generate_resolve_blob(StubId id, address destination);
  static RuntimeStub*   generate_throw_exception(StubId id, address runtime_entry);
 public:
  static void generate_initial_stubs(void);
  static void generate_stubs(void);
#if INCLUDE_JFR
  static void generate_jfr_stubs(void);
  // For c2: c_rarg0 is junk, call to runtime to write a checkpoint.
  // It returns a jobject handle to the event writer.
  // The handle is dereferenced and the return value is the event writer oop.
  static RuntimeStub* generate_jfr_write_checkpoint();
  // For c2: call to runtime to return a buffer lease.
  static RuntimeStub* generate_jfr_return_lease();
#endif
  static void init_adapter_library();

  static const char *stub_name(StubId id) {
    assert(StubInfo::is_shared(id), "not a shared stub %s", StubInfo::name(id));
    return StubInfo::name(id);
  }

  // max bytes for each dtrace string parameter
  enum { max_dtrace_string_size = 256 };

  // The following arithmetic routines are used on platforms that do
  // not have machine instructions to implement their functionality.
  // Do not remove these.

  // long arithmetics
  static jlong   lmul(jlong y, jlong x);
  static jlong   ldiv(jlong y, jlong x);
  static jlong   lrem(jlong y, jlong x);

  // float and double remainder
  static jfloat  frem(jfloat  x, jfloat  y);
  static jdouble drem(jdouble x, jdouble y);


#ifdef _WIN64
  // Workaround for fmod issue in the Windows x64 CRT
  static double fmod_winx64(double x, double y);
#endif

#ifdef __SOFTFP__
  static jfloat  fadd(jfloat x, jfloat y);
  static jfloat  fsub(jfloat x, jfloat y);
  static jfloat  fmul(jfloat x, jfloat y);
  static jfloat  fdiv(jfloat x, jfloat y);

  static jdouble dadd(jdouble x, jdouble y);
  static jdouble dsub(jdouble x, jdouble y);
  static jdouble dmul(jdouble x, jdouble y);
  static jdouble ddiv(jdouble x, jdouble y);
#endif // __SOFTFP__

  // float conversion (needs to set appropriate rounding mode)
  static jint    f2i (jfloat  x);
  static jlong   f2l (jfloat  x);
  static jint    d2i (jdouble x);
  static jlong   d2l (jdouble x);
  static jfloat  d2f (jdouble x);
  static jfloat  l2f (jlong   x);
  static jdouble l2d (jlong   x);
  static jfloat  i2f (jint    x);

#ifdef __SOFTFP__
  static jdouble i2d (jint    x);
  static jdouble f2d (jfloat  x);
#endif // __SOFTFP__

  // double trigonometrics and transcendentals
  static jdouble dsin(jdouble x);
  static jdouble dcos(jdouble x);
  static jdouble dtan(jdouble x);
  static jdouble dlog(jdouble x);
  static jdouble dlog10(jdouble x);
  static jdouble dexp(jdouble x);
  static jdouble dpow(jdouble x, jdouble y);

#if defined(__SOFTFP__) || defined(E500V2)
  static double dabs(double f);
#endif

#if defined(__SOFTFP__)
  static double dsqrt(double f);
#endif

  // Montgomery multiplication
  static void montgomery_multiply(jint *a_ints, jint *b_ints, jint *n_ints,
                                  jint len, jlong inv, jint *m_ints);
  static void montgomery_square(jint *a_ints, jint *n_ints,
                                jint len, jlong inv, jint *m_ints);

#ifdef __SOFTFP__
  // C++ compiler generates soft float instructions as well as passing
  // float and double in registers.
  static int  fcmpl(float x, float y);
  static int  fcmpg(float x, float y);
  static int  dcmpl(double x, double y);
  static int  dcmpg(double x, double y);

  static int unordered_fcmplt(float x, float y);
  static int unordered_dcmplt(double x, double y);
  static int unordered_fcmple(float x, float y);
  static int unordered_dcmple(double x, double y);
  static int unordered_fcmpge(float x, float y);
  static int unordered_dcmpge(double x, double y);
  static int unordered_fcmpgt(float x, float y);
  static int unordered_dcmpgt(double x, double y);

  static float  fneg(float f);
  static double dneg(double f);
#endif

  // exception handling across interpreter/compiler boundaries
  static address raw_exception_handler_for_return_address(JavaThread* current, address return_address);
  static address exception_handler_for_return_address(JavaThread* current, address return_address);

  // exception handling and implicit exceptions
  static address compute_compiled_exc_handler(nmethod* nm, address ret_pc, Handle& exception,
                                              bool force_unwind, bool top_frame_only, bool& recursive_exception_occurred);
  enum ImplicitExceptionKind {
    IMPLICIT_NULL,
    IMPLICIT_DIVIDE_BY_ZERO,
    STACK_OVERFLOW
  };
  static void    throw_AbstractMethodError(JavaThread* current);
  static void    throw_IncompatibleClassChangeError(JavaThread* current);
  static void    throw_ArithmeticException(JavaThread* current);
  static void    throw_NullPointerException(JavaThread* current);
  static void    throw_NullPointerException_at_call(JavaThread* current);
  static void    throw_StackOverflowError(JavaThread* current);
  static void    throw_delayed_StackOverflowError(JavaThread* current);
  static void    throw_StackOverflowError_common(JavaThread* current, bool delayed);
  static address continuation_for_implicit_exception(JavaThread* current,
                                                     address faulting_pc,
                                                     ImplicitExceptionKind exception_kind);

  // Post-slow-path-allocation, pre-initializing-stores step for
  // implementing e.g. ReduceInitialCardMarks
  static void on_slowpath_allocation_exit(JavaThread* current);

  static void enable_stack_reserved_zone(JavaThread* current);
  static frame look_for_reserved_stack_annotated_method(JavaThread* current, frame fr);

  // Shared stub locations
  static address get_poll_stub(address pc);

  static address get_ic_miss_stub() {
    assert(_ic_miss_blob!= nullptr, "oops");
    return _ic_miss_blob->entry_point();
  }

  static address get_handle_wrong_method_stub() {
    assert(_wrong_method_blob!= nullptr, "oops");
    return _wrong_method_blob->entry_point();
  }

  static address get_handle_wrong_method_abstract_stub() {
    assert(_wrong_method_abstract_blob!= nullptr, "oops");
    return _wrong_method_abstract_blob->entry_point();
  }

  static address get_resolve_opt_virtual_call_stub() {
    assert(_resolve_opt_virtual_call_blob != nullptr, "oops");
    return _resolve_opt_virtual_call_blob->entry_point();
  }
  static address get_resolve_virtual_call_stub() {
    assert(_resolve_virtual_call_blob != nullptr, "oops");
    return _resolve_virtual_call_blob->entry_point();
  }
  static address get_resolve_static_call_stub() {
    assert(_resolve_static_call_blob != nullptr, "oops");
    return _resolve_static_call_blob->entry_point();
  }

  static SafepointBlob* polling_page_return_handler_blob()     { return _polling_page_return_handler_blob; }
  static SafepointBlob* polling_page_safepoint_handler_blob()  { return _polling_page_safepoint_handler_blob; }
  static SafepointBlob* polling_page_vectors_safepoint_handler_blob()  { return _polling_page_vectors_safepoint_handler_blob; }

  static nmethod* cont_doYield_stub() {
    assert(_cont_doYield_stub != nullptr, "oops");
    return _cont_doYield_stub;
  }

  // Implicit exceptions
  static address throw_AbstractMethodError_entry()          {
    assert(_throw_AbstractMethodError_blob != nullptr, "");
    return _throw_AbstractMethodError_blob->entry_point();
  }
  static address throw_IncompatibleClassChangeError_entry() {
    assert(_throw_IncompatibleClassChangeError_blob != nullptr, "");
    return  _throw_IncompatibleClassChangeError_blob->entry_point();
  }
  static address throw_NullPointerException_at_call_entry() {
    assert(_throw_NullPointerException_at_call_blob != nullptr, "");
    return  _throw_NullPointerException_at_call_blob->entry_point();
  }
  static address throw_StackOverflowError_entry()           {
    assert(_throw_StackOverflowError_blob != nullptr, "");
    return _throw_StackOverflowError_blob->entry_point();
  }
  static address throw_delayed_StackOverflowError_entry()   {
    assert(_throw_delayed_StackOverflowError_blob != nullptr, "");
    return _throw_delayed_StackOverflowError_blob->entry_point();
  }

#if INCLUDE_JFR
  static address jfr_write_checkpoint() {
    assert(_jfr_write_checkpoint_blob != nullptr, "");
    return _jfr_write_checkpoint_blob->entry_point();
  }
  static address jfr_return_lease()     {
    assert(_jfr_return_lease_blob != nullptr, "");
    return _jfr_return_lease_blob->entry_point();
  }
#endif

  // Counters
#ifndef PRODUCT
  static address nof_megamorphic_calls_addr() { return (address)&_nof_megamorphic_calls; }
#endif // PRODUCT

  // Helper routine for full-speed JVMTI exception throwing support
  static void throw_and_post_jvmti_exception(JavaThread* current, Handle h_exception);
  static void throw_and_post_jvmti_exception(JavaThread* current, Symbol* name, const char *message = nullptr);

#if INCLUDE_JVMTI
  // Functions for JVMTI notifications
  static void notify_jvmti_vthread_start(oopDesc* vt, jboolean hide, JavaThread* current);
  static void notify_jvmti_vthread_end(oopDesc* vt, jboolean hide, JavaThread* current);
  static void notify_jvmti_vthread_mount(oopDesc* vt, jboolean hide, JavaThread* current);
  static void notify_jvmti_vthread_unmount(oopDesc* vt, jboolean hide, JavaThread* current);
#endif

  // RedefineClasses() tracing support for obsolete method entry
  static int rc_trace_method_entry(JavaThread* thread, Method* m);

  // To be used as the entry point for unresolved native methods.
  static address native_method_throw_unsatisfied_link_error_entry();

  static void register_finalizer(JavaThread* thread, oopDesc* obj);

  // dtrace notifications
  static int dtrace_object_alloc(oopDesc* o);
  static int dtrace_object_alloc(JavaThread* thread, oopDesc* o);
  static int dtrace_object_alloc(JavaThread* thread, oopDesc* o, size_t size);
  static int dtrace_method_entry(JavaThread* thread, Method* m);
  static int dtrace_method_exit(JavaThread* thread, Method* m);

  // Utility method for retrieving the Java thread id, returns 0 if the
  // thread is not a well formed Java thread.
  static jlong get_java_tid(JavaThread* thread);


  // used by native wrappers to re-enable yellow if overflow happened in native code
  static void reguard_yellow_pages();

  // Fill in the "X cannot be cast to a Y" message for ClassCastException
  //
  // @param thr the current thread
  // @param caster_klass the class of the object we are casting
  // @return the dynamically allocated exception message (must be freed
  // by the caller using a resource mark)
  //
  // BCP must refer to the current 'checkcast' opcode for the frame
  // on top of the stack.
  // The caller (or one of its callers) must use a ResourceMark
  // in order to correctly free the result.
  //
  static char* generate_class_cast_message(JavaThread* thr, Klass* caster_klass);

  // Fill in the "X cannot be cast to a Y" message for ClassCastException
  //
  // @param caster_klass the class of the object we are casting
  // @param target_klass the target klass attempt
  // @return the dynamically allocated exception message (must be freed
  // by the caller using a resource mark)
  //
  // This version does not require access the frame, so it can be called
  // from interpreted code
  // The caller (or one of it's callers) must use a ResourceMark
  // in order to correctly free the result.
  //
  static char* generate_class_cast_message(Klass* caster_klass, Klass* target_klass, Symbol* target_klass_name = nullptr);

  // Resolves a call site- may patch in the destination of the call into the
  // compiled code.
  static methodHandle resolve_helper(bool is_virtual, bool is_optimized, TRAPS);

 private:
  // deopt blob
  static void generate_deopt_blob(void);

  static bool handle_ic_miss_helper_internal(Handle receiver, nmethod* caller_nm, const frame& caller_frame,
                                             methodHandle callee_method, Bytecodes::Code bc, CallInfo& call_info,
                                             bool& needs_ic_stub_refill, TRAPS);

 public:
  static DeoptimizationBlob* deopt_blob(void)      { return _deopt_blob; }

  // Resets a call-site in compiled code so it will get resolved again.
  static methodHandle reresolve_call_site(TRAPS);

  // In the code prolog, if the klass comparison fails, the inline cache
  // misses and the call site is patched to megamorphic
  static methodHandle handle_ic_miss_helper(TRAPS);

  // Find the method that called us.
  static methodHandle find_callee_method(TRAPS);

  static void monitor_enter_helper(oopDesc* obj, BasicLock* lock, JavaThread* thread);

  static void monitor_exit_helper(oopDesc* obj, BasicLock* lock, JavaThread* current);

  // Issue UL warning for unlocked JNI monitor on virtual thread termination
  static void log_jni_monitor_still_held();

 private:
  static Handle find_callee_info(Bytecodes::Code& bc, CallInfo& callinfo, TRAPS);
  static Handle find_callee_info_helper(vframeStream& vfst, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS);

  static Method* extract_attached_method(vframeStream& vfst);

#if defined(X86) && defined(COMPILER1)
  // For Object.hashCode, System.identityHashCode try to pull hashCode from object header if available.
  static void inline_check_hashcode_from_object_header(MacroAssembler* masm, const methodHandle& method, Register obj_reg, Register result);
#endif // X86 && COMPILER1

 public:

  // Read the array of BasicTypes from a Java signature, and compute where
  // compiled Java code would like to put the results.  Values in reg_lo and
  // reg_hi refer to 4-byte quantities.  Values less than SharedInfo::stack0 are
  // registers, those above refer to 4-byte stack slots.  All stack slots are
  // based off of the window top.  SharedInfo::stack0 refers to the first usable
  // slot in the bottom of the frame. SharedInfo::stack0+1 refers to the memory word
  // 4-bytes higher.
  // return value is the maximum number of VMReg stack slots the convention will use.
  static int java_calling_convention(const BasicType* sig_bt, VMRegPair* regs, int total_args_passed);

  static void check_member_name_argument_is_last_argument(const methodHandle& method,
                                                          const BasicType* sig_bt,
                                                          const VMRegPair* regs) NOT_DEBUG_RETURN;

  // Ditto except for calling C
  //
  // C argument in register AND stack slot.
  // Some architectures require that an argument must be passed in a register
  // AND in a stack slot. These architectures provide a second VMRegPair array
  // to be filled by the c_calling_convention method. On other architectures,
  // null is being passed as the second VMRegPair array, so arguments are either
  // passed in a register OR in a stack slot.
  static int c_calling_convention(const BasicType *sig_bt, VMRegPair *regs, int total_args_passed);

  static int vector_calling_convention(VMRegPair *regs,
                                       uint num_bits,
                                       uint total_args_passed);

  // Generate I2C and C2I adapters. These adapters are simple argument marshalling
  // blobs. Unlike adapters in the tiger and earlier releases the code in these
  // blobs does not create a new frame and are therefore virtually invisible
  // to the stack walking code. In general these blobs extend the callers stack
  // as needed for the conversion of argument locations.

  // When calling a c2i blob the code will always call the interpreter even if
  // by the time we reach the blob there is compiled code available. This allows
  // the blob to pass the incoming stack pointer (the sender sp) in a known
  // location for the interpreter to record. This is used by the frame code
  // to correct the sender code to match up with the stack pointer when the
  // thread left the compiled code. In addition it allows the interpreter
  // to remove the space the c2i adapter allocated to do its argument conversion.

  // Although a c2i blob will always run interpreted even if compiled code is
  // present if we see that compiled code is present the compiled call site
  // will be patched/re-resolved so that later calls will run compiled.

  // Additionally a c2i blob need to have a unverified entry because it can be reached
  // in situations where the call site is an inlined cache site and may go megamorphic.

  // A i2c adapter is simpler than the c2i adapter. This is because it is assumed
  // that the interpreter before it does any call dispatch will record the current
  // stack pointer in the interpreter frame. On return it will restore the stack
  // pointer as needed. This means the i2c adapter code doesn't need any special
  // handshaking path with compiled code to keep the stack walking correct.

  static void generate_i2c2i_adapters(MacroAssembler *_masm,
                               int total_args_passed,
                               int max_arg,
                               const BasicType *sig_bt,
                               const VMRegPair *regs,
                               AdapterHandlerEntry* handler);

  static void gen_i2c_adapter(MacroAssembler *_masm,
                              int total_args_passed,
                              int comp_args_on_stack,
                              const BasicType *sig_bt,
                              const VMRegPair *regs);

  // OSR support

  // OSR_migration_begin will extract the jvm state from an interpreter
  // frame (locals, monitors) and store the data in a piece of C heap
  // storage. This then allows the interpreter frame to be removed from the
  // stack and the OSR nmethod to be called. That method is called with a
  // pointer to the C heap storage. This pointer is the return value from
  // OSR_migration_begin.

  static intptr_t* OSR_migration_begin(JavaThread *thread);

  // OSR_migration_end is a trivial routine. It is called after the compiled
  // method has extracted the jvm state from the C heap that OSR_migration_begin
  // created. It's entire job is to simply free this storage.
  static void OSR_migration_end(intptr_t* buf);

  // Convert a sig into a calling convention register layout
  // and find interesting things about it.
  static VMRegPair* find_callee_arguments(Symbol* sig, bool has_receiver, bool has_appendix, int *arg_size);
  static VMReg name_for_receiver();

  // "Top of Stack" slots that may be unused by the calling convention but must
  // otherwise be preserved.
  // On Intel these are not necessary and the value can be zero.
  // On Sparc this describes the words reserved for storing a register window
  // when an interrupt occurs.
  static uint out_preserve_stack_slots();

  // Stack slots that may be unused by the calling convention but must
  // otherwise be preserved.  On Intel this includes the return address.
  // On PowerPC it includes the 4 words holding the old TOC & LR glue.
  static uint in_preserve_stack_slots();

  static VMReg thread_register();

  static void continuation_enter_cleanup(MacroAssembler* masm);

  // Is vector's size (in bytes) bigger than a size saved by default?
  // For example, on x86 16 bytes XMM registers are saved by default.
  static bool is_wide_vector(int size);

  // Save and restore a native result
  static void    save_native_result(MacroAssembler *_masm, BasicType ret_type, int frame_slots);
  static void restore_native_result(MacroAssembler *_masm, BasicType ret_type, int frame_slots);

  // Generate a native wrapper for a given method.  The method takes arguments
  // in the Java compiled code convention, marshals them to the native
  // convention (handlizes oops, etc), transitions to native, makes the call,
  // returns to java state (possibly blocking), unhandlizes any result and
  // returns.
  //
  // The wrapper may contain special-case code if the given method
  // is a compiled method handle adapter, such as _invokeBasic, _linkToVirtual, etc.
  static nmethod* generate_native_wrapper(MacroAssembler* masm,
                                          const methodHandle& method,
                                          int compile_id,
                                          BasicType* sig_bt,
                                          VMRegPair* regs,
                                          BasicType ret_type);

  // A compiled caller has just called the interpreter, but compiled code
  // exists.  Patch the caller so he no longer calls into the interpreter.
  static void fixup_callers_callsite(Method* moop, address ret_pc);
  static bool should_fixup_call_destination(address destination, address entry_point, address caller_pc, Method* moop, CodeBlob* cb);

  // Slow-path Locking and Unlocking
  static void complete_monitor_locking_C(oopDesc* obj, BasicLock* lock, JavaThread* current);
  static void complete_monitor_unlocking_C(oopDesc* obj, BasicLock* lock, JavaThread* current);

  // Resolving of calls
  static address get_resolved_entry        (JavaThread* current, methodHandle callee_method);
  static address resolve_static_call_C     (JavaThread* current);
  static address resolve_virtual_call_C    (JavaThread* current);
  static address resolve_opt_virtual_call_C(JavaThread* current);

  // arraycopy, the non-leaf version.  (See StubRoutines for all the leaf calls.)
  static void slow_arraycopy_C(oopDesc* src,  jint src_pos,
                               oopDesc* dest, jint dest_pos,
                               jint length, JavaThread* thread);

  // handle ic miss with caller being compiled code
  // wrong method handling (inline cache misses)
  static address handle_wrong_method(JavaThread* current);
  static address handle_wrong_method_abstract(JavaThread* current);
  static address handle_wrong_method_ic_miss(JavaThread* current);

  static address handle_unsafe_access(JavaThread* thread, address next_pc);

#ifndef PRODUCT

  // Collect and print inline cache miss statistics
 private:
  enum { maxICmiss_count = 100 };
  static int     _ICmiss_index;                  // length of IC miss histogram
  static int     _ICmiss_count[maxICmiss_count]; // miss counts
  static address _ICmiss_at[maxICmiss_count];    // miss addresses
  static void trace_ic_miss(address at);

 public:
  static uint _ic_miss_ctr;                      // total # of IC misses
  static uint _wrong_method_ctr;
  static uint _resolve_static_ctr;
  static uint _resolve_virtual_ctr;
  static uint _resolve_opt_virtual_ctr;
  static uint _implicit_null_throws;
  static uint _implicit_div0_throws;

  static uint _jbyte_array_copy_ctr;       // Slow-path byte array copy
  static uint _jshort_array_copy_ctr;      // Slow-path short array copy
  static uint _jint_array_copy_ctr;        // Slow-path int array copy
  static uint _jlong_array_copy_ctr;       // Slow-path long array copy
  static uint _oop_array_copy_ctr;         // Slow-path oop array copy
  static uint _checkcast_array_copy_ctr;   // Slow-path oop array copy, with cast
  static uint _unsafe_array_copy_ctr;      // Slow-path includes alignment checks
  static uint _generic_array_copy_ctr;     // Slow-path includes type decoding
  static uint _slow_array_copy_ctr;        // Slow-path failed out to a method call

  static uint _unsafe_set_memory_ctr;      // Slow-path includes alignment checks

  static uint _new_instance_ctr;           // 'new' object requires GC
  static uint _new_array_ctr;              // 'new' array requires GC
  static uint _multi2_ctr, _multi3_ctr, _multi4_ctr, _multi5_ctr;
  static uint _find_handler_ctr;           // find exception handler
  static uint _rethrow_ctr;                // rethrow exception
  static uint _mon_enter_stub_ctr;         // monitor enter stub
  static uint _mon_exit_stub_ctr;          // monitor exit stub
  static uint _mon_enter_ctr;              // monitor enter slow
  static uint _mon_exit_ctr;               // monitor exit slow
  static uint _partial_subtype_ctr;        // SubRoutines::partial_subtype_check

  // Statistics code
  // stats for "normal" compiled calls (non-interface)
  static int64_t _nof_normal_calls;               // total # of calls
  static int64_t _nof_inlined_calls;              // total # of inlined normal calls
  static int64_t _nof_static_calls;               // total # of calls to static methods or super methods (invokespecial)
  static int64_t _nof_inlined_static_calls;       // total # of inlined static calls
  // stats for compiled interface calls
  static int64_t _nof_interface_calls;            // total # of compiled calls
  static int64_t _nof_inlined_interface_calls;    // total # of inlined interface calls

 public: // for compiler
  static address nof_normal_calls_addr()                { return (address)&_nof_normal_calls; }
  static address nof_inlined_calls_addr()               { return (address)&_nof_inlined_calls; }
  static address nof_static_calls_addr()                { return (address)&_nof_static_calls; }
  static address nof_inlined_static_calls_addr()        { return (address)&_nof_inlined_static_calls; }
  static address nof_interface_calls_addr()             { return (address)&_nof_interface_calls; }
  static address nof_inlined_interface_calls_addr()     { return (address)&_nof_inlined_interface_calls; }
  static void print_call_statistics(uint64_t comp_total);
  static void print_ic_miss_histogram();

#endif // PRODUCT

  static void print_statistics() PRODUCT_RETURN;
};


// ---------------------------------------------------------------------------
// Implementation of AdapterHandlerLibrary
//
// This library manages argument marshaling adapters and native wrappers.
// There are 2 flavors of adapters: I2C and C2I.
//
// The I2C flavor takes a stock interpreted call setup, marshals the
// arguments for a Java-compiled call, and jumps to Rmethod-> code()->
// code_begin().  It is broken to call it without an nmethod assigned.
// The usual behavior is to lift any register arguments up out of the
// stack and possibly re-pack the extra arguments to be contiguous.
// I2C adapters will save what the interpreter's stack pointer will be
// after arguments are popped, then adjust the interpreter's frame
// size to force alignment and possibly to repack the arguments.
// After re-packing, it jumps to the compiled code start.  There are
// no safepoints in this adapter code and a GC cannot happen while
// marshaling is in progress.
//
// The C2I flavor takes a stock compiled call setup plus the target method in
// Rmethod, marshals the arguments for an interpreted call and jumps to
// Rmethod->_i2i_entry.  On entry, the interpreted frame has not yet been
// setup.  Compiled frames are fixed-size and the args are likely not in the
// right place.  Hence all the args will likely be copied into the
// interpreter's frame, forcing that frame to grow.  The compiled frame's
// outgoing stack args will be dead after the copy.
//
// Native wrappers, like adapters, marshal arguments.  Unlike adapters they
// also perform an official frame push & pop.  They have a call to the native
// routine in their middles and end in a return (instead of ending in a jump).
// The native wrappers are stored in real nmethods instead of the BufferBlobs
// used by the adapters.  The code generation happens here because it's very
// similar to what the adapters have to do.

class AdapterHandlerEntry : public MetaspaceObj {
  friend class AdapterHandlerLibrary;

 public:
  static const int ENTRIES_COUNT = 4;

 private:
  AdapterFingerPrint* _fingerprint;
  address _i2c_entry;
  address _c2i_entry;
  address _c2i_unverified_entry;
  address _c2i_no_clinit_check_entry;
  bool    _linked;

  static const char *_entry_names[];

#ifdef ASSERT
  // Captures code and signature used to generate this adapter when
  // verifying adapter equivalence.
  unsigned char* _saved_code;
  int            _saved_code_length;
#endif

  AdapterHandlerEntry(AdapterFingerPrint* fingerprint) :
    _fingerprint(fingerprint),
    _i2c_entry(nullptr),
    _c2i_entry(nullptr),
    _c2i_unverified_entry(nullptr),
    _c2i_no_clinit_check_entry(nullptr),
    _linked(false)
#ifdef ASSERT
    , _saved_code(nullptr),
    _saved_code_length(0)
#endif
  { }

  ~AdapterHandlerEntry();

  // Allocate on CHeap instead of metaspace (see JDK-8331086).
  // Dummy argument is used to avoid C++ warning about using
  // deleted opearator MetaspaceObj::delete().
  void* operator new(size_t size, size_t dummy) throw() {
    assert(size == BytesPerWord * heap_word_size(sizeof(AdapterHandlerEntry)), "should match");
    void* p = AllocateHeap(size, mtCode);
    memset(p, 0, size);
    return p;
  }

 public:
  static AdapterHandlerEntry* allocate(AdapterFingerPrint* fingerprint) {
    return new(0) AdapterHandlerEntry(fingerprint);
  }

  static void deallocate(AdapterHandlerEntry *handler) {
    handler->~AdapterHandlerEntry();
  }

  void set_entry_points(address i2c_entry, address c2i_entry, address c2i_unverified_entry, address c2i_no_clinit_check_entry, bool linked = true) {
    _i2c_entry = i2c_entry;
    _c2i_entry = c2i_entry;
    _c2i_unverified_entry = c2i_unverified_entry;
    _c2i_no_clinit_check_entry = c2i_no_clinit_check_entry;
    _linked = linked;
  }

  address get_i2c_entry()                  const { return _i2c_entry; }
  address get_c2i_entry()                  const { return _c2i_entry; }
  address get_c2i_unverified_entry()       const { return _c2i_unverified_entry; }
  address get_c2i_no_clinit_check_entry()  const { return _c2i_no_clinit_check_entry; }

  static const char* entry_name(int i) {
    assert(i >=0 && i < ENTRIES_COUNT, "entry id out of range");
    return _entry_names[i];
  }

  bool is_linked() const { return _linked; }
  address base_address();
  void relocate(address new_base);

  AdapterFingerPrint* fingerprint() const { return _fingerprint; }

#ifdef ASSERT
  // Used to verify that code generated for shared adapters is equivalent
  void save_code   (unsigned char* code, int length);
  bool compare_code(AdapterHandlerEntry* other);
#endif

  //virtual void print_on(outputStream* st) const;  DO NOT USE
  void print_adapter_on(outputStream* st) const;

  void metaspace_pointers_do(MetaspaceClosure* it);
  int size() const {return (int)heap_word_size(sizeof(AdapterHandlerEntry)); }
  MetaspaceObj::Type type() const { return AdapterHandlerEntryType; }

  void remove_unshareable_info() NOT_CDS_RETURN;
  void link() NOT_CDS_RETURN;
};

#if INCLUDE_CDS
class ArchivedAdapterTable;
#endif // INCLUDE_CDS

class AdapterHandlerLibrary: public AllStatic {
  friend class SharedRuntime;
 private:
  static BufferBlob* _buffer; // the temporary code buffer in CodeCache
  static AdapterHandlerEntry* _abstract_method_handler;
  static AdapterHandlerEntry* _no_arg_handler;
  static AdapterHandlerEntry* _int_arg_handler;
  static AdapterHandlerEntry* _obj_arg_handler;
  static AdapterHandlerEntry* _obj_int_arg_handler;
  static AdapterHandlerEntry* _obj_obj_arg_handler;
#if INCLUDE_CDS
  static ArchivedAdapterTable _aot_adapter_handler_table;
#endif // INCLUDE_CDS

  static BufferBlob* buffer_blob();
  static void initialize();
  static AdapterHandlerEntry* get_simple_adapter(const methodHandle& method);
  static AdapterBlob* lookup_aot_cache(AdapterHandlerEntry* handler);
  static AdapterHandlerEntry* create_adapter(AdapterBlob*& new_adapter,
                                             int total_args_passed,
                                             BasicType* sig_bt,
                                             bool is_transient = false);
  static void create_abstract_method_handler();
  static void lookup_simple_adapters() NOT_CDS_RETURN;
#ifndef PRODUCT
  static void print_adapter_handler_info(outputStream* st, AdapterHandlerEntry* handler, AdapterBlob* adapter_blob);
#endif // PRODUCT
 public:

  static AdapterHandlerEntry* new_entry(AdapterFingerPrint* fingerprint);
  static void create_native_wrapper(const methodHandle& method);
  static AdapterHandlerEntry* get_adapter(const methodHandle& method);
  static AdapterHandlerEntry* lookup(int total_args_passed, BasicType* sig_bt);
  static bool generate_adapter_code(AdapterBlob*& adapter_blob,
                                    AdapterHandlerEntry* handler,
                                    int total_args_passed,
                                    BasicType* sig_bt,
                                    bool is_transient);

#ifdef ASSERT
  static void verify_adapter_sharing(int total_args_passed, BasicType* sig_bt, AdapterHandlerEntry* cached);
#endif // ASSERT

  static void print_handler(const CodeBlob* b) { print_handler_on(tty, b); }
  static void print_handler_on(outputStream* st, const CodeBlob* b);
  static bool contains(const CodeBlob* b);
  static const char* name(AdapterFingerPrint* fingerprint);
  static uint32_t id(AdapterFingerPrint* fingerprint);
#ifndef PRODUCT
  static void print_statistics();
#endif // PRODUCT

  static bool is_abstract_method_adapter(AdapterHandlerEntry* adapter);

  static AdapterBlob* link_aot_adapter_handler(AdapterHandlerEntry* handler) NOT_CDS_RETURN_(nullptr);
  static void dump_aot_adapter_table() NOT_CDS_RETURN;
  static void serialize_shared_table_header(SerializeClosure* soc) NOT_CDS_RETURN;
  static void link_aot_adapters() NOT_CDS_RETURN;
};

#endif // SHARE_RUNTIME_SHAREDRUNTIME_HPP
