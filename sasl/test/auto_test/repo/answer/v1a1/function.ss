; ModuleID = 'prog'

define i32 @foo(i32 %a) {
.entry:
  ret i32 %a
                                                  ; No predecessors!
  ret i32 0
}
