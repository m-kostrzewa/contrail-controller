/*
 * Copyright (c) 2017 Juniper Networks, Inc. All rights reserved.
 */

#include <cassert>

#include "base/logging.h"
#include "../pkt0_interface.h"

const LPCTSTR PKT0_PATH = TEXT("\\\\.\\vrouterPkt0");

void Pkt0Interface::InitControlInterface() {
    pkt_handler()->agent()->set_pkt_interface_name(name_);

    const DWORD access_flags = GENERIC_READ | GENERIC_WRITE;
    const DWORD attrs = OPEN_EXISTING;
    const DWORD flags = FILE_FLAG_OVERLAPPED;

    HANDLE handle = CreateFile(PKT0_PATH, access_flags, 0, NULL, attrs, flags, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        LOG(ERROR, "Error while opening Pkt0 pipe: " << GetFormattedWindowsErrorMsg());
        assert(0);
    }

    boost::system::error_code ec;
    input_.assign(handle, ec);
    assert(ec == 0);

    VrouterControlInterface::InitControlInterface();
    AsyncRead();
}

void Pkt0Interface::SendImpl(uint8_t *buff, uint16_t buff_len, const PacketBufferPtr &pkt,
                             buffer_list& buffers) {
    auto collected_data = std::vector<uint8_t>(boost::asio::buffer_size(buffers));
    boost::asio::buffer_copy(boost::asio::buffer(collected_data), buffers);
    auto collected_buffer = boost::asio::buffer(collected_data.data(), collected_data.size());

    boost::asio::async_write(input_, collected_buffer,
                             boost::bind(&Pkt0Interface::WriteHandler, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred,
                                         pkt, buff));
}

void Pkt0RawInterface::InitControlInterface() {
}
