;=========================== begin_copyright_notice ============================
;
; Copyright (c) 2014-2021 Intel Corporation
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom
; the Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included
; in all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;
;============================ end_copyright_notice =============================

; RUN: igc_opt -igc-replace-unsupported-intrinsics -verify -S %s -o %t
; RUN: FileCheck %s < %t

target triple = "igil_32_GEN9"

%struct.data_t = type { [111 x i16] }

; Function Attrs: nounwind
define void @test_kernel(%struct.data_t addrspace(1)* %p, %struct.data_t addrspace(1)* %q) #0 {
; CHECK: [[pIV:%[a-zA-Z0-9_]+]] = alloca i32
; CHECK: [[vSrc:%[a-zA-Z0-9_]+]] = bitcast %struct.data_t addrspace(1)* %q to <16 x i16> addrspace(1)*
; CHECK: [[vDst:%[a-zA-Z0-9_]+]] = bitcast %struct.data_t addrspace(1)* %p to <16 x i16> addrspace(1)*
; CHECK: memcpy.body:
; CHECK:   [[IV:%[a-zA-Z0-9_]+]] = load i32, i32* [[pIV]]
; CHECK:   [[GEP0:%[a-zA-Z0-9_]+]] = getelementptr <16 x i16>, <16 x i16> addrspace(1)* [[vSrc]], i32 [[IV]]
; CHECK:   [[GEP1:%[a-zA-Z0-9_]+]] = getelementptr <16 x i16>, <16 x i16> addrspace(1)* [[vDst]], i32 [[IV]]
; CHECK:   [[LD:%[a-zA-Z0-9_]+]] = load <16 x i16>, <16 x i16> addrspace(1)* [[GEP0]], align 2
; CHECK:   store <16 x i16> [[LD]], <16 x i16> addrspace(1)* [[GEP1]], align 2
; CHECK:   [[INC0:%[a-zA-Z0-9_]+]] = add i32 [[IV]], 1
; CHECK:   store i32 [[INC0]], i32* [[pIV]]
; CHECK:   [[CMP:%[a-zA-Z0-9_]+]] = icmp ult i32 [[INC0]], 6
; CHECK:   br i1 [[CMP]], label %memcpy.body, label %memcpy.post
; CHECK: memcpy.post:
; CHECK:   [[memcpy_src:%[a-zA-Z0-9_]+]] = bitcast %struct.data_t addrspace(1)* %q to i8 addrspace(1)*
; CHECK:   [[memcpy_dst:%[a-zA-Z0-9_]+]] = bitcast %struct.data_t addrspace(1)* %p to i8 addrspace(1)*
; CHECK:   [[GEP2:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_src]], i32 192
; CHECK:   [[GEP3:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_dst]], i32 192
; CHECK:   [[REM0:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP2]] to <8 x i16> addrspace(1)*
; CHECK:   [[REM1:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP3]] to <8 x i16> addrspace(1)*
; CHECK:   [[L0:%[_a-zA-Z0-9_]+]] = load <8 x i16>, <8 x i16> addrspace(1)* [[REM0]], align 2
; CHECK:   store <8 x i16> [[L0]], <8 x i16> addrspace(1)* [[REM1]], align 2
; CHECK:   [[GEP4:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_src]], i32 208
; CHECK:   [[GEP5:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_dst]], i32 208
; CHECK:   [[REM2:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP4]] to <4 x i16> addrspace(1)*
; CHECK:   [[REM3:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP5]] to <4 x i16> addrspace(1)*
; CHECK:   [[L1:%[_a-zA-Z0-9_]+]] = load <4 x i16>, <4 x i16> addrspace(1)* [[REM2]], align 2
; CHECK:   store <4 x i16> [[L1]], <4 x i16> addrspace(1)* [[REM3]], align 2
; CHECK:   [[GEP6:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_src]], i32 216
; CHECK:   [[GEP7:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_dst]], i32 216
; CHECK:   [[REM4:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP6]] to <2 x i16> addrspace(1)*
; CHECK:   [[REM5:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP7]] to <2 x i16> addrspace(1)*
; CHECK:   [[L2:%[_a-zA-Z0-9_]+]] = load <2 x i16>, <2 x i16> addrspace(1)* [[REM4]], align 2
; CHECK:   store <2 x i16> [[L2]], <2 x i16> addrspace(1)* [[REM5]], align 2
; CHECK:   [[GEP8:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_src]], i32 220
; CHECK:   [[GEP9:%[_a-zA-Z0-9_]+]] = getelementptr i8, i8 addrspace(1)* [[memcpy_dst]], i32 220
; CHECK:   [[REM6:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP8]] to i16 addrspace(1)*
; CHECK:   [[REM7:%[_a-zA-Z0-9_]+]] = bitcast i8 addrspace(1)* [[GEP9]] to i16 addrspace(1)*
; CHECK:   [[L3:%[_a-zA-Z0-9_]+]] = load i16, i16 addrspace(1)* [[REM6]], align 2
; CHECK:   store i16 [[L3]], i16 addrspace(1)* [[REM7]], align 2
; CHECK:   ret void
  %Dst = bitcast %struct.data_t addrspace(1)* %p to i8 addrspace(1)*
  %Src = bitcast %struct.data_t addrspace(1)* %q to i8 addrspace(1)*
  call void @llvm.memcpy.p1i8.p1i8.i32(i8 addrspace(1)* %Dst, i8 addrspace(1)* %Src, i32 222, i32 2, i1 false)
  ret void
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p1i8.p1i8.i32(i8 addrspace(1)* nocapture, i8 addrspace(1)* nocapture, i32, i32, i1) #0

attributes #0 = { nounwind }