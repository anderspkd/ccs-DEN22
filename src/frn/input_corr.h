#ifndef _FRN_INPUT_CORR_H
#define _FRN_INPUT_CORR_H

#include <memory>
#include <vector>

#include "frn/network.h"
#include "frn/shr.h"

namespace frn {

Field GetRandomElement(frn::lib::primitives::PRG& prg);

frn::lib::primitives::PRG FieldElementToPrg(const Field& element);

class InputSetup {
 public:
  class Correlator;

  InputSetup(std::shared_ptr<Network> network,
             frn::lib::secret_sharing::Replicator<Field> replicator,
             frn::lib::primitives::PRG prg)
      : mNetwork(network), mReplicator(replicator), mPrg(prg){};

  InputSetup::Correlator Run();

  class Correlator {
   public:
    Correlator(std::vector<std::vector<frn::lib::primitives::PRG>> prgs,
               std::vector<frn::lib::primitives::PRG> prg, std::size_t share_size)
        : mPrgs(prgs), mPrg(prg), mShareSize(share_size){};

    /**
     * @brief Returns r_j where j is this party's ID.
     */
    Field GetMask() {
      frn::Field v;
      for (auto& prg : mPrg) v += GetRandomElement(prg);
      return v;
    };

    /**
     * @brief Returns [r_id] for some id.
     * @param id the id
     */
    Shr GetMaskShare(unsigned id) {
      std::vector<frn::Field> share;
      share.reserve(mShareSize);
      auto prg_id = mPrgs[id];
      for (std::size_t i = 0; i < mShareSize; i++)
        share.emplace_back(GetRandomElement(prg_id[i]));
      return share;
    };

   private:
    std::vector<std::vector<frn::lib::primitives::PRG>> mPrgs;
    std::vector<frn::lib::primitives::PRG> mPrg;
    std::size_t mShareSize;
  };

 private:
  std::shared_ptr<Network> mNetwork;
  frn::lib::secret_sharing::Replicator<Field> mReplicator;
  frn::lib::primitives::PRG mPrg;
};

}  // namespace frn

#endif /* _FRN_INPUT_CORR_H */
