; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -mergeicmps -mtriple=x86_64-unknown-unknown -S | FileCheck %s

%S = type { i32, i32 }

define zeroext i1 @opeq(
; CHECK-LABEL: @opeq(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[FIRST_I:%.*]] = getelementptr inbounds [[S:%.*]], %S* [[A:%.*]], i64 0, i32 0
; CHECK-NEXT:    [[TMP0:%.*]] = load i32, i32* [[FIRST_I]], align 4
; CHECK-NEXT:    [[FIRST1_I:%.*]] = getelementptr inbounds [[S]], %S* [[B:%.*]], i64 0, i32 0
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, i32* [[FIRST1_I]], align 4
; CHECK-NEXT:    [[CMP_I:%.*]] = icmp eq i32 [[TMP0]], [[TMP1]]
; CHECK-NEXT:    br i1 [[CMP_I]], label [[LAND_RHS_I:%.*]], label [[OPEQ1_EXIT:%.*]]
; CHECK:       land.rhs.i:
; CHECK-NEXT:    [[SECOND_I:%.*]] = getelementptr inbounds [[S]], %S* [[A]], i64 0, i32 1
; CHECK-NEXT:    [[TMP2:%.*]] = load atomic i32, i32* [[SECOND_I]] seq_cst, align 4
; CHECK-NEXT:    [[SECOND2_I:%.*]] = getelementptr inbounds [[S]], %S* [[B]], i64 0, i32 1
; CHECK-NEXT:    [[TMP3:%.*]] = load i32, i32* [[SECOND2_I]], align 4
; CHECK-NEXT:    [[CMP3_I:%.*]] = icmp eq i32 [[TMP2]], [[TMP3]]
; CHECK-NEXT:    br label [[OPEQ1_EXIT]]
; CHECK:       opeq1.exit:
; CHECK-NEXT:    [[TMP4:%.*]] = phi i1 [ false, [[ENTRY:%.*]] ], [ [[CMP3_I]], [[LAND_RHS_I]] ]
; CHECK-NEXT:    ret i1 [[TMP4]]
;
  %S* nocapture readonly dereferenceable(8) %a,
  %S* nocapture readonly dereferenceable(8) %b) local_unnamed_addr #0 {
entry:
  %first.i = getelementptr inbounds %S, %S* %a, i64 0, i32 0
  %0 = load i32, i32* %first.i, align 4
  %first1.i = getelementptr inbounds %S, %S* %b, i64 0, i32 0
  %1 = load i32, i32* %first1.i, align 4
  %cmp.i = icmp eq i32 %0, %1
  br i1 %cmp.i, label %land.rhs.i, label %opeq1.exit

land.rhs.i:
  %second.i = getelementptr inbounds %S, %S* %a, i64 0, i32 1
  %2 = load atomic i32, i32* %second.i seq_cst, align 4
  %second2.i = getelementptr inbounds %S, %S* %b, i64 0, i32 1
  %3 = load i32, i32* %second2.i, align 4
  %cmp3.i = icmp eq i32 %2, %3
  br label %opeq1.exit

opeq1.exit:
  %4 = phi i1 [ false, %entry ], [ %cmp3.i, %land.rhs.i ]
  ret i1 %4
}