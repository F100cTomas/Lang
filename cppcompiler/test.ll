; ModuleID = 'putchar.cpp'
source_filename = "putchar.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone sspstrong uwtable
define dso_local i64 @my_getchar(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca i8, align 1
  store i64 %0, ptr %2, align 8
  call void asm sideeffect "mov $$0, %rax\0A\09mov $$0, %rdi\0A\09lea $0, %rsi\0A\09mov $$1, %rdx\0A\09syscall", "*m,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i8) %3) #1, !srcloc !6
  %4 = load i8, ptr %3, align 1
  %5 = sext i8 %4 to i64
  ret i64 %5
}

; Function Attrs: mustprogress noinline nounwind optnone sspstrong uwtable
define dso_local i64 @my_putchar(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  call void asm sideeffect "lea $0, %rsi\0A\09mov $$1, %rax\0A\09mov $$1, %rdi\0A\09mov $$1, %rdx\0A\09syscall", "*m,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory},~{dirflag},~{fpsr},~{flags}"(ptr elementtype(i64) %2) #1, !srcloc !7
  ret i64 0
}

attributes #0 = { mustprogress noinline nounwind optnone sspstrong uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"clang version 21.1.4"}
!6 = !{i64 102, i64 118, i64 171, i64 219, i64 258}
!7 = !{i64 467, i64 485, i64 528, i64 582, i64 629}
