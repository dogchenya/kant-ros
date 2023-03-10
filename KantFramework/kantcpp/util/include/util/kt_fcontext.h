#ifndef _KT_CONTEXT_FCONTEXT_H_
#define _KT_CONTEXT_FCONTEXT_H_

namespace kant {

typedef void* fcontext_t;

struct transfer_t {
  fcontext_t fctx;
  void* data;
};

extern "C" transfer_t jump_fcontext(fcontext_t const to, void* vp);
extern "C" fcontext_t make_fcontext(void* sp, std::size_t size, void (*fn)(transfer_t));

}  // namespace kant

#endif
