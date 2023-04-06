#ifndef _KT_CONTEXT_FCONTEXT_H_
#define _KT_CONTEXT_FCONTEXT_H_

namespace kant {

typedef void* fcontext_t;

struct transfer_t {
  fcontext_t fctx;
  void* data;
};

/**
   * 在两个上下文之间进行切换。
   * 在协程或纤程的上下文切换中，有两个上下文：当前上下文和目标上下文。当前上下文是指当前正在执行的协程或纤程的上下文，而目标上下文是指即将要执行的协程或纤程的上下文。
   * @param to 是目标上下文的指针
   * @param vp 是传递给目标上下文的参数
   * @return: 该函数返回值是一个 void* 类型的指针，指向返回到当前上下文时传递的值。
   */
extern "C" transfer_t jump_fcontext(fcontext_t const to, void* vp);

/**
   * 用于创建一个新的协程上下文并返回一个 fcontext_t 类型的结构体。
   * @param sp 指向新协程堆栈的指针，注意：栈指针（SP）必须对齐到 sizeof(void*) 的倍数。
   * @param size 新协程堆栈的大小，单位为字节。
   * @param fn 协程函数指针，它是一个 void(*) (transfer_t) 类型的函数指针，表示协程的入口函数。transfer_t 是一个结构体类型，用于协程之间的数据传递。当协程切换到这个新的上下文时，它会从这个函数开始执行。
   * @return: 新的协程fcontext_t 类型的结构体
   */
extern "C" fcontext_t make_fcontext(void* sp, std::size_t size, void (*fn)(transfer_t));

}  // namespace kant

#endif
