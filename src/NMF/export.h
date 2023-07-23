#pragma once

#ifdef NMF_EXPORTS
#define NMF_EXPORT __declspec(dllexport)
#else
#define NMF_EXPORT __declspec(dllimport)
#endif

#define NMF_DELETE_ALL_INIT(type) \
    type() = delete; \
    type(const type&) = delete; \
    type(type&&) = delete; \
    type& operator=(const type&) = delete; \
    type& operator=(type&&) = delete;

#define NMF_DELETE_INIT(type) \
    type(const type&) = delete; \
    type(type&&) = delete; \
    type& operator=(const type&) = delete; \
    type& operator=(type&&) = delete;
