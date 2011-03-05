; ModuleID = 'prog'

define i32 @foo(i32 %a) {
"1_1":
  ret i32 %a
                                                  ; No predecessors!
  ret i32 0
}
