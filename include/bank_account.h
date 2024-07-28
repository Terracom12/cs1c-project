#pragma once

#include "money_type.h"
#include "monthly_statement.h"
#include "util/date_util.h"

#include <memory>
#include <string_view>
#include <vector>

template <typename T>
concept BankAccountConcept = std::copyable<T> && requires(const T constAcc, T acc) {
    { constAcc.getAccountName() } -> std::convertible_to<std::string_view>;
    { constAcc.getAccountNumber() } -> std::convertible_to<int>;
    { constAcc.getAccountOpeningDate() } -> std::convertible_to<Date>;
    { constAcc.getBalance() } -> std::convertible_to<Money>;
    { constAcc.getMonthlyStatement(Date{}) } -> std::same_as<const MonthlyStatement&>;
    { constAcc.getAllMonthlyStatements() } -> std::same_as<const std::vector<MonthlyStatement>&>;

    { acc.deposit(Money{}) } -> std::same_as<void>;
    { acc.withdraw(Money{}) } -> std::same_as<void>;
};

class BankAccount
{
public:
    template <typename AccountType>
        requires(!std::same_as<AccountType, BankAccount>) && BankAccountConcept<AccountType>
    /*implicit*/ BankAccount(const AccountType& account) // NOLINT
        : pimpl_(std::make_unique<Model<AccountType>>(account)) {}

    BankAccount(const BankAccount& other)
        : pimpl_(other.pimpl_->clone()) {}

    BankAccount& operator=(const BankAccount& rhs) {
        if (&rhs == this) {
            return *this;
        }
        pimpl_.reset(rhs.pimpl_.get());
        return *this;
    }
    BankAccount(BankAccount&&) = default;
    BankAccount& operator=(BankAccount&&) = default;
    ~BankAccount() = default;

    std::string_view getAccountName() const { return pimpl_->getAccountName(); };
    int getAccountNumber() const { return pimpl_->getAccountNumber(); };
    Date getAccountOpeningDate() const { return pimpl_->getAccountOpeningDate(); };
    Money getBalance() const { return pimpl_->getBalance(); };
    const MonthlyStatement& getMonthlyStatement(const Date& when) const { return pimpl_->getMonthlyStatement(when); };
    const std::vector<MonthlyStatement>& getAllMonthlyStatements() const { return pimpl_->getAllMonthlyStatements(); };
    void deposit(const Money& amount) { pimpl_->deposit(amount); };
    void withdraw(const Money& amount) { pimpl_->withdraw(amount); };

    friend class CheckingAccount;

private:
    class Concept
    {
    public:
        virtual ~Concept() = default;
        virtual std::unique_ptr<Concept> clone() = 0;

        virtual std::string_view getAccountName() const = 0;
        virtual int getAccountNumber() const = 0;
        virtual Date getAccountOpeningDate() const = 0;
        virtual Money getBalance() const = 0;
        virtual const MonthlyStatement& getMonthlyStatement(const Date& when) const = 0;
        virtual const std::vector<MonthlyStatement>& getAllMonthlyStatements() const = 0;
        virtual void deposit(const Money& amount) = 0;
        virtual void withdraw(const Money& amount) = 0;
    };

    template <BankAccountConcept AccountType>
    class Model : public virtual Concept
    {
    public:
        explicit Model(const AccountType& account)
            : impl_(account) {}
        std::unique_ptr<Concept> clone() override { return std::make_unique<Model>(*this); }

        std::string_view getAccountName() const override { return impl_.getAccountName(); };
        int getAccountNumber() const override { return impl_.getAccountNumber(); };
        Date getAccountOpeningDate() const override { return impl_.getAccountOpeningDate(); };
        Money getBalance() const override { return impl_.getBalance(); };
        const MonthlyStatement& getMonthlyStatement(const Date& when) const override {
            return impl_.getMonthlyStatement(when);
        };
        const std::vector<MonthlyStatement>& getAllMonthlyStatements() const override {
            return impl_.getAllMonthlyStatements();
        };
        void deposit(const Money& amount) override { impl_.deposit(amount); };
        void withdraw(const Money& amount) override { impl_.withdraw(amount); };

    protected:
        AccountType impl_; // NOLINT
    };

    std::unique_ptr<Concept> pimpl_;
};
