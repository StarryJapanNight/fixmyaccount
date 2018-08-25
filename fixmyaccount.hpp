/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/currency.hpp>


namespace eosiosystem
{
class system_contract;
}

namespace eosio
{

using std::string;

class fixmyaccount : public contract
{

  public:
    // Constructor
    fixmyaccount(account_name self) : contract(self) {}

    // Payment (transfer) action
    void payment(const currency::transfer &transfer_data);

    // Apply (main) function
    void apply(const account_name contract, const account_name act);

    auto get_contractfees()
    {
        return contractfees;
    }

  private:

    // Account to transfer fees to
    const account_name contractfees = N(namedaccount);

};

} // namespace eosio
