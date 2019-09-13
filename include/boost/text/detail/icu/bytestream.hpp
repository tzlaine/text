// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
// Copyright (C) 2009-2012, International Business Machines
// Corporation and others. All Rights Reserved.
//
// Copyright 2007 Google Inc. All Rights Reserved.
// Author: sanjay@google.com (Sanjay Ghemawat)
//
// Abstract interface that consumes a sequence of bytes (ByteSink).
//
// Used so that we can write a single piece of code that can operate
// on a variety of output string types.
//
// Various implementations of this interface are provided:
//   ByteSink:
//      CheckedArrayByteSink    Write to a flat array, with bounds checking
//      StringByteSink          Write to an STL string

// This code is a contribution of Google code, and the style used here is
// a compromise between the original Google code and the ICU coding guidelines.
// For example, data types are ICU-ified (size_t,int->int32_t),
// and API comments doxygen-ified, but function names and behavior are
// as in the original, if possible.
// Assertion-style error handling, not available in ICU, was changed to
// parameter "pinning" similar to UnicodeString.
//
// In addition, this is only a partial port of the original Google code,
// limited to what was needed so far. The (nearly) complete original code
// is in the ICU svn repository at icuhtml/trunk/design/strings/contrib
// (see ICU ticket 6765, r25517).

#ifndef BYTESTREAM_H_
#define BYTESTREAM_H_

/**
 * \file
 * \brief C++ API: Interface for writing bytes, and implementation classes.
 */

#include "utypes.hpp"


namespace boost { namespace text { namespace detail { namespace icu {

    /**
     * A ByteSink can be filled with bytes.
     * @stable ICU 4.2
     */
    class ByteSink
    {
    public:
        /**
         * Default constructor.
         * @stable ICU 4.2
         */
        ByteSink() {}
        /**
         * Virtual destructor.
         * @stable ICU 4.2
         */
        virtual ~ByteSink() {}

        /**
         * Append "bytes[0,n-1]" to this.
         * @param bytes the pointer to the bytes
         * @param n the number of bytes; must be non-negative
         * @stable ICU 4.2
         */
        virtual void Append(const char * bytes, int32_t n) = 0;

        /**
         * Returns a writable buffer for appending and writes the buffer's
         * capacity to *result_capacity. Guarantees
         * *result_capacity>=min_capacity. May return a pointer to the
         * caller-owned scratch buffer which must have
         * scratch_capacity>=min_capacity.
         * The returned buffer is only valid until the next operation
         * on this ByteSink.
         *
         * After writing at most *result_capacity bytes, call Append() with the
         * pointer returned from this function and the number of bytes written.
         * Many Append() implementations will avoid copying bytes if this
         * function returned an internal buffer.
         *
         * Partial usage example:
         *  int32_t capacity;
         *  char* buffer = sink->GetAppendBuffer(..., &capacity);
         *  ... Write n bytes into buffer, with n <= capacity.
         *  sink->Append(buffer, n);
         * In many implementations, that call to Append will avoid copying
         * bytes.
         *
         * If the ByteSink allocates or reallocates an internal buffer, it
         * should use the desired_capacity_hint if appropriate. If a caller
         * cannot provide a reasonable guess at the desired capacity, it should
         * pass desired_capacity_hint=0.
         *
         * If a non-scratch buffer is returned, the caller may only pass
         * a prefix to it to Append().
         * That is, it is not correct to pass an interior pointer to Append().
         *
         * The default implementation always returns the scratch buffer.
         *
         * @param min_capacity required minimum capacity of the returned buffer;
         *                     must be non-negative
         * @param desired_capacity_hint desired capacity of the returned buffer;
         *                              must be non-negative
         * @param scratch default caller-owned buffer
         * @param scratch_capacity capacity of the scratch buffer
         * @param result_capacity pointer to an integer which will be set to the
         *                        capacity of the returned buffer
         * @return a buffer with *result_capacity>=min_capacity
         * @stable ICU 4.2
         */
        virtual char * GetAppendBuffer(
            int32_t min_capacity,
            int32_t desired_capacity_hint,
            char * scratch,
            int32_t scratch_capacity,
            int32_t * result_capacity)
        {
            if (min_capacity < 1 || scratch_capacity < min_capacity) {
                *result_capacity = 0;
                return nullptr;
            }
            *result_capacity = scratch_capacity;
            return scratch;
        }

        /**
         * Flush internal buffers.
         * Some byte sinks use internal buffers or provide buffering
         * and require calling Flush() at the end of the stream.
         * The ByteSink should be ready for further Append() calls after
         * Flush(). The default implementation of Flush() does nothing.
         * @stable ICU 4.2
         */
        virtual void Flush() {}

    private:
        ByteSink(const ByteSink &) = delete;
        ByteSink & operator=(const ByteSink &) = delete;
    };

}}}}

#endif
