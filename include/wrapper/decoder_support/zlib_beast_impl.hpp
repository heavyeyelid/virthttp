/* Copyright (c) 2019 HeavyEyelid */
/* Distributed under the Apache 2 License */
/* Official repository: https://github.com/HeavyEyelid/virthttp */
/* This is a derivative work based on Boost.Beast, copyright below: */
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//
// This is a derivative work based on Zlib, copyright below:
/*
    Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    Jean-loup Gailly        Mark Adler
    jloup@gzip.org          madler@alumni.caltech.edu

    The data format used by the zlib library is described by RFCs (Request for
    Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
    (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
*/

#pragma once
#include <array>
#include "zlib_beast_detail.hpp"

/** Raw deflate stream decompressor.

    This implements a raw deflate stream decompressor. The deflate
    protocol is a compression protocol described in
    "DEFLATE Compressed Data Format Specification version 1.3"
    located here: https://tools.ietf.org/html/rfc1951

    The implementation is a refactored port to C++ of ZLib's "inflate".
    A more detailed description of ZLib is at http://zlib.net/.

    Compression can be done in a single step if the buffers are large
    enough (for example if an input file is memory mapped), or can be done
    by repeated calls of the compression function. In the latter case, the
    application must provide more input and/or consume the output (providing
    more output space) before each call.
*/
class InflateStream : private detail::inflate_stream {
  public:
    /** Construct a raw deflate decompression stream.

        The window size is set to the default of 15 bits.
    */
    InflateStream() = default;

    /** Reset the stream.

        This puts the stream in a newly constructed state with
        the previously specified window size, but without de-allocating
        any dynamically created structures.
    */
    void reset() { doReset(); }

    /** Reset the stream.

        This puts the stream in a newly constructed state with the
        specified window size, but without de-allocating any dynamically
        created structures.
    */
    void reset(int windowBits) { doReset(windowBits); }

    /** Put the stream in a newly constructed state.

        All dynamically allocated memory is de-allocated.
    */
    void clear() { doClear(); }

    /** Decompress input and produce output.

        This function decompresses as much data as possible, and stops when
        the input buffer becomes empty or the output buffer becomes full. It
        may introduce some output latency (reading input without producing any
        output) except when forced to flush.

        One or both of the following actions are performed:

        @li Decompress more input starting at `zs.next_in` and update `zs.next_in`
        and `zs.avail_in` accordingly. If not all input can be processed (because
        there is not enough room in the output buffer), `zs.next_in` is updated
        and processing will resume at this point for the next call.

        @li Provide more output starting at `zs.next_out` and update `zs.next_out`
        and `zs.avail_out` accordingly. `write` provides as much output as
        possible, until there is no more input data or no more space in the output
        buffer (see below about the flush parameter).

        Before the call, the application should ensure that at least one of the
        actions is possible, by providing more input and/or consuming more output,
        and updating the values in `zs` accordingly. The application can consume
        the uncompressed output when it wants, for example when the output buffer
        is full (`zs.avail_out == 0`), or after each call. If `write` returns no
        error and with zero `zs.avail_out`, it must be called again after making
        room in the output buffer because there might be more output pending.

        The flush parameter may be `Flush::none`, `Flush::sync`, `Flush::finish`,
        `Flush::block`, or `Flush::trees`. `Flush::sync` requests to flush as much
        output as possible to the output buffer. `Flush::block` requests to stop if
        and when it gets to the next deflate block boundary. When decoding the
        zlib or gzip format, this will cause `write` to return immediately after
        the header and before the first block. When doing a raw inflate, `write` will
        go ahead and process the first block, and will return when it gets to the
        end of that block, or when it runs out of data.

        The `Flush::block` option assists in appending to or combining deflate
        streams. Also to assist in this, on return `write` will set `zs.data_type`
        to the number of unused bits in the last byte taken from `zs.next_in`, plus
        64 if `write` is currently decoding the last block in the deflate stream,
        plus 128 if `write` returned immediately after decoding an end-of-block code
        or decoding the complete header up to just before the first byte of the
        deflate stream. The end-of-block will not be indicated until all of the
        uncompressed data from that block has been written to `zs.next_out`. The
        number of unused bits may in general be greater than seven, except when
        bit 7 of `zs.data_type` is set, in which case the number of unused bits
        will be less than eight. `zs.data_type` is set as noted here every time
        `write` returns for all flush options, and so can be used to determine the
        amount of currently consumed input in bits.

        The `Flush::trees` option behaves as `Flush::block` does, but it also returns
        when the end of each deflate block header is reached, before any actual data
        in that block is decoded. This allows the caller to determine the length of
        the deflate block header for later use in random access within a deflate block.
        256 is added to the value of `zs.data_type` when `write` returns immediately
        after reaching the end of the deflate block header.

        `write` should normally be called until it returns `error::end_of_stream` or
        another error. However if all decompression is to be performed in a single
        step (a single call of `write`), the parameter flush should be set to
        `Flush::finish`. In this case all pending input is processed and all pending
        output is flushed; `zs.avail_out` must be large enough to hold all of the
        uncompressed data for the operation to complete. (The size of the uncompressed
        data may have been saved by the compressor for this purpose.) The use of
        `Flush::finish` is not required to perform an inflation in one step. However
        it may be used to inform inflate that a faster approach can be used for the
        single call. `Flush::finish` also informs inflate to not maintain a sliding
        window if the stream completes, which reduces inflate's memory footprint.
        If the stream does not complete, either because not all of the stream is
        provided or not enough output space is provided, then a sliding window will be
        allocated and `write` can be called again to continue the operation as if
        `Flush::none` had been used.

        In this implementation, `write` always flushes as much output as possible to
        the output buffer, and always uses the faster approach on the first call. So
        the effects of the flush parameter in this implementation are on the return value
        of `write` as noted below, when `write` returns early when `Flush::block` or
        `Flush::trees` is used, and when `write` avoids the allocation of memory for a
        sliding window when `Flush::finsih` is used.

        If a preset dictionary is needed after this call,
        `write` sets `zs.adler` to the Adler-32 checksum of the dictionary chosen by
        the compressor and returns `error::need_dictionary`; otherwise it sets
        `zs.adler` to the Adler-32 checksum of all output produced so far (that is,
        `zs.total_out bytes`) and returns no error, `error::end_of_stream`, or an
        error code as described below. At the end of the stream, `write` checks that
        its computed adler32 checksum is equal to that saved by the compressor and
        returns `error::end_of_stream` only if the checksum is correct.

        This function returns no error if some progress has been made (more input
        processed or more output produced), `error::end_of_stream` if the end of the
        compressed data has been reached and all uncompressed output has been produced,
        `error::need_dictionary` if a preset dictionary is needed at this point,
        `error::invalid_data` if the input data was corrupted (input stream not
        conforming to the zlib format or incorrect check value), `error::stream_error`
        if the stream structure was inconsistent (for example if `zs.next_in` or
        `zs.next_out` was null), `error::need_buffers` if no progress is possible or
        if there was not enough room in the output buffer when `Flush::finish` is
        used. Note that `error::need_buffers` is not fatal, and `write` can be called
        again with more input and more output space to continue decompressing.
    */
    void write(boost::beast::zlib::z_params& zs, boost::beast::zlib::Flush flush, boost::system::error_code& ec) { doWrite(zs, flush, ec); }
};

/** Raw deflate compressor.

    This is a port of zlib's "deflate" functionality to C++.
*/
class DeflateStream : private detail::deflate_stream {
  public:
    /** Construct a default deflate stream.

        Upon construction, the stream settings will be set
        to these default values:

        @li `level = 6`

        @li `windowBits = 15`

        @li `memLevel = 8`

        @li `strategy = Strategy::normal`

        Although the stream is ready to be used immediately
        after construction, any required internal buffers are
        not dynamically allocated until needed.
    */
    DeflateStream() { reset(6, 15, DEF_MEM_LEVEL, boost::beast::zlib::Strategy::normal); }

    /** Reset the stream and compression settings.

        This function initializes the stream to the specified
        compression settings.

        Although the stream is ready to be used immediately
        after a reset, any required internal buffers are not
        dynamically allocated until needed.

        @note Any unprocessed input or pending output from
        previous calls are discarded.
    */
    void reset(int level, int windowBits, int memLevel, boost::beast::zlib::Strategy strategy) { doReset(level, windowBits, memLevel, strategy); }

    /** Reset the stream without deallocating memory.

        This function performs the equivalent of calling `clear`
        followed by `reset` with the same compression settings,
        without deallocating the internal buffers.

        @note Any unprocessed input or pending output from
        previous calls are discarded.
    */
    void reset() { doReset(); }

    /** Clear the stream.

        This function resets the stream and frees all dynamically
        allocated internal buffers. The compression settings are
        left unchanged.

        @note Any unprocessed input or pending output from
        previous calls are discarded.
    */
    void clear() { doClear(); }

    /** Returns the upper limit on the size of a compressed block.

        This function makes a conservative estimate of the maximum number
        of bytes needed to store the result of compressing a block of
        data based on the current compression level and strategy.

        @param sourceLen The size of the uncompressed data.

        @return The maximum number of resulting compressed bytes.
    */
    std::size_t upper_bound(std::size_t sourceLen) const { return doUpperBound(sourceLen); }

    /** Fine tune internal compression parameters.

        Compression parameters should only be tuned by someone who
        understands the algorithm used by zlib's deflate for searching
        for the best matching string, and even then only by the most
        fanatic optimizer trying to squeeze out the last compressed bit
        for their specific input data. Read the deflate.c source code
        (ZLib) for the meaning of the max_lazy, good_length, nice_length,
        and max_chain parameters.
    */
    void tune(int good_length, int max_lazy, int nice_length, int max_chain) { doTune(good_length, max_lazy, nice_length, max_chain); }

    /** Compress input and write output.

        This function compresses as much data as possible, and stops when
        the input buffer becomes empty or the output buffer becomes full.
        It may introduce some output latency (reading input without
        producing any output) except when forced to flush.

        In each call, one or both of these actions are performed:

        @li Compress more input starting at `zs.next_in` and update
        `zs.next_in` and `zs.avail_in` accordingly. If not all
        input can be processed (because there is not enough room in
        the output buffer), `zs.next_in` and `zs.avail_in` are updated
        and processing will resume at this point for the next call.

        @li Provide more output starting at `zs.next_out` and update
        `zs.next_out` and `zs.avail_out` accordingly. This action is
        forced if the parameter flush is not `Flush::none`. Forcing
        flush frequently degrades the compression ratio, so this parameter
        should be set only when necessary (in interactive applications).
        Some output may be provided even if flush is not set.

        Before the call, the application must ensure that at least one
        of the actions is possible, by providing more input and/or
        consuming more output, and updating `zs.avail_in` or `zs.avail_out`
        accordingly; `zs.avail_out` should never be zero before the call.
        The application can consume the compressed output when it wants,
        for example when the output buffer is full (`zs.avail_out == 0`),
        or after each call of `write`. If `write` returns no error
        with zero `zs.avail_out`, it must be called again after making
        room in the output buffer because there might be more output
        pending.

        Normally the parameter flush is set to `Flush::none`, which allows
        deflate to decide how much data to accumulate before producing
        output, in order to maximize compression.

        If the parameter flush is set to `Flush::sync`, all pending output
        is flushed to the output buffer and the output is aligned on a
        byte boundary, so that the decompressor can get all input data
        available so far. In particular `zs.avail_in` is zero after the
        call if enough output space has been provided before the call.
        Flushing may degrade compression for some compression algorithms
        and so it should be used only when necessary. This completes the
        current deflate block and follows it with an empty stored block
        that is three bits plus filler bits to the next byte, followed
        by the four bytes `{ 0x00, 0x00 0xff 0xff }`.

        If flush is set to `Flush::partial`, all pending output is flushed
        to the output buffer, but the output is not aligned to a byte
        boundary. All of the input data so far will be available to the
        decompressor, as for Z_SYNC_FLUSH. This completes the current
        deflate block and follows it with an empty fixed codes block that
        is 10 bits long. This assures that enough bytes are output in order
        for the decompressor to finish the block before the empty fixed
        code block.

        If flush is set to `Flush::block`, a deflate block is completed
        and emitted, as for `Flush::sync`, but the output is not aligned
        on a byte boundary, and up to seven bits of the current block are
        held to be written as the next byte after the next deflate block
        is completed. In this case, the decompressor may not be provided
        enough bits at this point in order to complete decompression of
        the data provided so far to the compressor. It may need to wait
        for the next block to be emitted. This is for advanced applications
        that need to control the emission of deflate blocks.

        If flush is set to `Flush::full`, all output is flushed as with
        `Flush::sync`, and the compression state is reset so that
        decompression can restart from this point if previous compressed
        data has been damaged or if random access is desired. Using
        `Flush::full` too often can seriously degrade compression.

        If `write` returns with `zs.avail_out == 0`, this function must
        be called again with the same value of the flush parameter and
        more output space (updated `zs.avail_out`), until the flush is
        complete (`write` returns with non-zero `zs.avail_out`). In the
        case of a `Flush::full`or `Flush::sync`, make sure that
        `zs.avail_out` is greater than six to avoid repeated flush markers
        due to `zs.avail_out == 0` on return.

        If the parameter flush is set to `Flush::finish`, pending input
        is processed, pending output is flushed and deflate returns the
        error `error::end_of_stream` if there was enough output space;
        if deflate returns with no error, this function must be called
        again with `Flush::finish` and more output space (updated
        `zs.avail_out`) but no more input data, until it returns the
        error `error::end_of_stream` or another error. After `write` has
        returned the `error::end_of_stream` error, the only possible
        operations on the stream are to reset or destroy.

        `Flush::finish` can be used immediately after initialization
        if all the compression is to be done in a single step. In this
        case, `zs.avail_out` must be at least value returned by
        `upper_bound` (see below). Then `write` is guaranteed to return
        the `error::end_of_stream` error. If not enough output space
        is provided, deflate will not return `error::end_of_stream`,
        and it must be called again as described above.

        `write` returns no error if some progress has been made (more
        input processed or more output produced), `error::end_of_stream`
        if all input has been consumed and all output has been produced
        (only when flush is set to `Flush::finish`), `error::stream_error`
        if the stream state was inconsistent (for example if `zs.next_in`
        or `zs.next_out` was `nullptr`), `error::need_buffers` if no
        progress is possible (for example `zs.avail_in` or `zs.avail_out`
        was zero). Note that `error::need_buffers` is not fatal, and
        `write` can be called again with more input and more output space
        to continue compressing.
    */
    void write(boost::beast::zlib::z_params& zs, boost::beast::zlib::Flush flush, boost::system::error_code& ec) { doWrite(zs, flush, ec); }

    /** Update the compression level and strategy.

        This function dynamically updates the compression level and
        compression strategy. The interpretation of level and strategy
        is as in @ref reset. This can be used to switch between compression
        and straight copy of the input data, or to switch to a different kind
        of input data requiring a different strategy. If the compression level
        is changed, the input available so far is compressed with the old level
        (and may be flushed); the new level will take effect only at the next
        call of @ref write.

        Before the call of `params`, the stream state must be set as for a
        call of @ref write, since the currently available input may have to be
        compressed and flushed. In particular, `zs.avail_out` must be non-zero.

        @return `Z_OK` if success, `Z_STREAM_ERROR` if the source stream state
        was inconsistent or if a parameter was invalid, `error::need_buffers`
        if `zs.avail_out` was zero.
    */
    void params(boost::beast::zlib::z_params& zs, int level, boost::beast::zlib::Strategy strategy, boost::system::error_code& ec) {
        doParams(zs, level, strategy, ec);
    }

    /** Return bits pending in the output.

        This function returns the number of bytes and bits of output
        that have been generated, but not yet provided in the available
        output. The bytes not provided would be due to the available
        output space having being consumed. The number of bits of output
        not provided are between 0 and 7, where they await more bits to
        join them in order to fill out a full byte. If pending or bits
        are `nullptr`, then those values are not set.

        @return `Z_OK` if success, or `Z_STREAM_ERROR` if the source
        stream state was inconsistent.
    */
    void pending(unsigned* value, int* bits) { doPending(value, bits); }

    /** Insert bits into the compressed output stream.

        This function inserts bits in the deflate output stream. The
        intent is that this function is used to start off the deflate
        output with the bits leftover from a previous deflate stream when
        appending to it. As such, this function can only be used for raw
        deflate, and must be used before the first `write` call after an
        initialization. `bits` must be less than or equal to 16, and that
        many of the least significant bits of `value` will be inserted in
        the output.

        @return `error::need_buffers` if there was not enough room in
        the internal buffer to insert the bits.
    */
    void prime(int bits, int value, boost::system::error_code& ec) { return doPrime(bits, value, ec); }
};

/** Returns the upper limit on the size of a compressed block.

    This function makes a conservative estimate of the maximum number
    of bytes needed to store the result of compressing a block of
    data.

    @param bytes The size of the uncompressed data.

    @return The maximum number of resulting compressed bytes.
*/
std::size_t deflate_upper_bound(std::size_t bytes);

/*  For the default windowBits of 15 and memLevel of 8, this function returns
    a close to exact, as well as small, upper bound on the compressed size.
    They are coded as constants here for a reason--if the #define's are
    changed, then this function needs to be changed as well.  The return
    value for 15 and 8 only works for those exact settings.

    For any setting other than those defaults for windowBits and memLevel,
    the value returned is a conservative worst case for the maximum expansion
    resulting from using fixed blocks instead of stored blocks, which deflate
    can emit on compressed data for some combinations of the parameters.

    This function could be more sophisticated to provide closer upper bounds for
    every combination of windowBits and memLevel.  But even the conservative
    upper bound of about 14% expansion does not seem onerous for output buffer
    allocation.
*/
inline std::size_t deflate_upper_bound(std::size_t bytes) { return bytes + ((bytes + 7) >> 3) + ((bytes + 63) >> 6) + 5 + 6; }
