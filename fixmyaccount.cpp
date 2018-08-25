/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include "fixmyaccount.hpp"

namespace eosio
{


// Action: Buy an account listed for sale
void fixmyaccount::payment(const currency::transfer &transfer_data)
{


    // Ignore transfers from the contract or our funding account.
    // This is necessary to allow transfers of EOS to the contract.
    if (transfer_data.from == _self || transfer_data.from == N(namedaccount))
    {
        return;
    }

    // ----------------------------------------------
    // Valid transaction checks
    // ----------------------------------------------

    // Check the transfer is valid
    eosio_assert(transfer_data.quantity.symbol == string_to_symbol(4, "EOS"), "Payment Error: You must pay in EOS.");
    eosio_assert(transfer_data.quantity.is_valid(), "Payment Error: Quantity is not valid.");
    
    // 1.0 EOS service fee
    auto service_fee = asset(10000); // 1.0000 EOS
    eosio_assert(transfer_data.quantity == service_fee, "Payment Error: Service fee is 1.0000 EOS.");

    // Check the buy memo is valid
    eosio_assert(transfer_data.memo.length() <= 12, "Payment Error: Transfer memo must contain the name of the account you wish to fix.");

    // Extract account to buy from memo
    const string account_string = transfer_data.memo.substr(0, 12);
    const account_name account_to_fix = string_to_name(account_string.c_str());

    // Check an account name has been listed in the memo
    eosio_assert(is_account(account_to_fix), "Payment Error: The account specified doesn't exist.");
    eosio_assert(account_to_fix != N(), "Payment Error: You must specify an account name to fix in the memo.");
    eosio_assert(account_to_fix != _self, "Payment Error: The contract cannot fix itself!.");

    // ----------------------------------------------
    // Delegate bandwidth to account_to_fix
    // ----------------------------------------------

    // Delgate CPU (0.5 EOS) and NET (0.1 EOS) from contract
    eosio::asset del_amount_cpu(5000);
    eosio::asset del_amount_net(1000);

    action(
        permission_level{_self, N(owner)},
        N(eosio), N(delegatebw),
        std::make_tuple(_self, account_to_fix, del_amount_net, del_amount_cpu,true))
        .send();

    // ----------------------------------------------
    // Buy ram for account_to_fix
    // ----------------------------------------------

    // Amount of ram to buy (0.2 EOS)
    eosio::asset buyram_amount(2000);

    action(
        permission_level{_self, N(owner)},
        N(eosio), N(buyram),
        std::make_tuple(_self,account_to_fix, buyram_amount))
        .send();

    // ----------------------------------------------
    // Contract fees
    // ----------------------------------------------

    // Contract fee (0.2 EOS)
    eosio::asset contract_fee(2000);

    // Transfer EOS from contract to contract fees account
    action(
        permission_level{_self, N(owner)},
        N(eosio.token), N(transfer),
        std::make_tuple(_self, get_contractfees(), contract_fee, std::string("FIXMYACCOUNT: Contract fee for " + name{account_to_fix}.to_string())))
        .send();

}


void fixmyaccount::apply(const account_name contract, const account_name act)
{

    switch (act)
    {
    case N(transfer):

        if (contract == N(eosio.token))
        {
            payment(unpack_action_data<currency::transfer>());
        }
        break;
    default:
        break;
    }
}

extern "C"
{
    [[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        fixmyaccount _fixmyaccount(receiver);

        switch (action)
        {
        case N(transfer):
            if (code == N(eosio.token))
            {
                _fixmyaccount.payment(unpack_action_data<currency::transfer>());
            }
            break;
        default:
            break;
        }
        eosio_exit(0);
    }
}

} // namespace eosio
