#ifndef _FRN_TCP_NETWORK_H
#define _FRN_TCP_NETWORK_H

#include <memory>

#include "frn/lib/logging.h"
#include "frn/lib/net/builder.h"
#include "frn/lib/net/network.h"
#include "frn/network.h"

namespace frn {

class TcpNetwork final : public Network {
 public:
  /**
   * @brief Create a TCP network where all parties run on localhost.
   * @param id the ID of this party
   * @param n the number of parties
   * @param with_logger whether to log stuff for the underlying network
   */
  static std::shared_ptr<TcpNetwork> CreateWithLocalParties(
      unsigned id, std::size_t n, unsigned base_port, bool with_logger = true) {
    auto logger =
        frn::lib::logging::create_logger<frn::lib::logging::StdoutLogger>(true);
    auto builder = frn::lib::net::Network::Builder();
    builder = builder.LocalPeerId(id)
                  .TransportType(frn::lib::net::Network::TransportType::eTcp)
                  .Size(n)
                  .BasePort(base_port)
                  .AllPartiesLocal();
    if (with_logger) builder = builder.Logger(logger);
    frn::lib::secret_sharing::Replicator<Field> rep(n, (n - 1) / 3);
    if (with_logger) logger->Info("created network for %", id);
    return std::shared_ptr<TcpNetwork>(
        new TcpNetwork(id, n, builder.Build(), logger, rep));
  };

  TcpNetwork() = delete;

  ~TcpNetwork(){

  };

  void Connect() { mNetwork.Connect(); };

  void Close() { mNetwork.Close(); };

  void Send(unsigned id, const std::vector<Field>& values) override {
    auto n = values.size() * Field::ByteSize();
    auto buffer = std::make_unique<unsigned char[]>(n);
    auto ptr = buffer.get();
    for (const auto& v : values) {
      v.ToBytes(ptr);
      ptr += Field::ByteSize();
    }
    mSummary.Send(id, n);
    mNetwork.SendTo(id, buffer.get(), n);
  };

  void SendShares(unsigned id, const std::vector<Shr>& shares) override {
    for (const auto& shr : shares)
      Send(id, static_cast<const std::vector<Field>>(shr));
  };

  void SendBytes(unsigned id, const std::vector<unsigned char>& data) override {
    mSummary.Send(id, data.size());
    mNetwork.SendTo(id, data.data(), data.size());
  };

  std::vector<Field> Recv(unsigned id, std::size_t n) override {
    auto m = n * Field::ByteSize();
    auto buffer = std::make_unique<unsigned char[]>(m);
    mSummary.Recv(id, m);
    mNetwork.RecvFrom(id, buffer.get(), m);
    std::vector<Field> values;
    values.reserve(n);
    auto ptr = buffer.get();
    for (std::size_t i = 0; i < n; i++) {
      values.emplace_back(Field::FromBytes(ptr));
      ptr += Field::ByteSize();
    }
    return values;
  };

  std::vector<Shr> RecvShares(unsigned id, std::size_t n) override {
    std::vector<Shr> values;
    values.reserve(n);
    for (std::size_t i = 0; i < n; i++) {
      values.emplace_back(Recv(id, mReplicator.ShareSize()));
    }
    return values;
  };

  std::vector<unsigned char> RecvBytes(unsigned id, std::size_t n) override {
    std::vector<unsigned char> r(n);
    mSummary.Recv(id, n);
    mNetwork.RecvFrom(id, r.data(), n);
    return r;
  };

  void PrintCommunicationSummary() const {
    std::cout << "communication summary for " << this->Id() << ":\n";
    mSummary.Print();
    std::cout << "\n";
  };

 private:
  class Summary {
   public:
    Summary(std::size_t n)
        : mSent(std::vector<std::size_t>(n, 0)),
          mRecv(std::vector<std::size_t>(n, 0)){};

    void Send(unsigned id, std::size_t n) { mSent[id] += n; };

    void Recv(unsigned id, std::size_t n) { mRecv[id] += n; };

    void Print() const {
      for (std::size_t i = 0; i < mSent.size(); i++) {
        if (mSent[i] && mRecv[i]) {
          std::cout << "sent/received to/from " << i << ": ";
          std::cout << mSent[i] << "/" << mRecv[i] << " bytes\n";
        } else if (mSent[i]) {
          std::cout << "sent to " << i << ": ";
          std::cout << mSent[i] << " bytes\n";
        } else if (mRecv[i]) {
          std::cout << "received from " << i << ": ";
          std::cout << mRecv[i] << " bytes\n";
        }
      }
    }

   private:
    std::vector<std::size_t> mSent;
    std::vector<std::size_t> mRecv;
  };

  TcpNetwork(unsigned id, std::size_t n, frn::lib::net::Network&& network,
             std::shared_ptr<frn::lib::logging::Logger> logger,
             frn::lib::secret_sharing::Replicator<Field>& replicator)
      : Network(id, n),
        mNetwork(std::move(network)),
        mLogger(logger),
        mReplicator(replicator),
        mSummary(Summary(n)){};

  frn::lib::net::Network mNetwork;
  std::shared_ptr<frn::lib::logging::Logger> mLogger;
  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  Summary mSummary;
};

}  // namespace frn

#endif /* _FRN_TCP_NETWORK_H */
