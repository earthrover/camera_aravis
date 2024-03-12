#pragma once

#ifndef CAMERA_ARAVIS_INTERNAL_GPTR_H
#define CAMERA_ARAVIS_INTERNAL_GPTR_H

#include <utility>

#include <glib.h>

namespace camera_aravis {

    template<typename T>
    class NonOwnedGPtr;

    template<typename T>
    class GPtr {
        public:
        GPtr() = default;
        GPtr(std::nullptr_t): ptr(nullptr) {};

        explicit GPtr(T* pointer, bool take_copy = false) {
            if (take_copy) {
                ptr = static_cast<T*>(g_object_ref(pointer));
            } else {
                ptr = pointer;
            }
        }

        GPtr(const GPtr<T>& other): ptr(static_cast<T*>(g_object_ref(other.ptr))) {}

        GPtr(GPtr<T>&& other) noexcept: ptr(std::exchange(other.ptr, nullptr)) {}

        GPtr<T>& operator=(const GPtr<T>& other) {
            *this = GPtr<T>(other);
            return *this;
        }

        GPtr<T>& operator=(GPtr<T>&& other) noexcept {
            std::swap(ptr, other.ptr);
            return *this;
        }


        ~GPtr() { g_object_unref(ptr); }

        T* operator->() const noexcept { return get(); }
        T& operator*() const noexcept { return *get(); }

        T* get() const { return ptr; }

        explicit operator bool() const { return nullptr != get(); }

        private:
        T* ptr = nullptr;

        friend NonOwnedGPtr<T>;
    };

    template<typename T>
    class NonOwnedGPtr {
        public:
        NonOwnedGPtr() = default;
        NonOwnedGPtr(std::nullptr_t): ptr(nullptr) {};

        explicit NonOwnedGPtr(T* pointer) { ptr = pointer; }

        NonOwnedGPtr(const GPtr<T>& gptr) { ptr = gptr.ptr; }


        T* operator->() const noexcept { return get(); }
        T& operator*() const noexcept { return *get(); }

        T* get() const { return ptr; }

        explicit operator bool() const { return nullptr != get(); }

        private:
        T* ptr = nullptr;
    };
}  // namespace camera_aravis


#endif
