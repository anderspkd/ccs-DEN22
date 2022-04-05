#include "frn/lib/net/channel.h"

#include <future>
#include <memory>
#include <vector>

#include "frn/lib/net/connector.h"

void frn::lib::net::AsyncSenderChannel::Open() {
  mConnector->Connect();
  mSender = std::async(std::launch::async, [&]() {
    while (true) {
      std::vector<unsigned char> buf = mSendQueue.Front();
      if (mConnector->State() != Connector::State::eActive) break;
      mConnector->Send(buf.data(), buf.size());
      mSendQueue.PopFront();
    }
  });
}

void frn::lib::net::AsyncSenderChannel::Send(const unsigned char* buffer,
                                        std::size_t size) {
  mSendQueue.PushBack({buffer, buffer + size});
}

void frn::lib::net::AsyncSenderChannel::Close() {
  // signal the async job that we're done by closing the connector and then
  // sending an empty message. The message ensures that sender job will check
  // if the connector is still alive and thus exit correctly.
  mConnector->Close();
  unsigned char dummy = 1;
  Send(&dummy, 1);
  mSender.wait();
}
