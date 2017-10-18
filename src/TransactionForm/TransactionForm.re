open Helpers;

loadCSS "./TransactionForm.css";

module BuyForm = {
  let component = ReasonReact.statelessComponent "BuyForm";
  let make ::cashInput ::cryptoInput ::receivedInput ::spendInput _children => {
    ...component,
    render: fun _self =>
      <Aux> receivedInput cryptoInput spendInput cashInput </Aux>
  };
};

module SellForm = {
  let component = ReasonReact.statelessComponent "SellForm";
  let make ::cashInput ::cryptoInput ::receivedInput ::spendInput _children => {
    ...component,
    render: fun _self =>
      <Aux> receivedInput cashInput spendInput cryptoInput </Aux>
  };
};

module DepositForm = {
  let component = ReasonReact.statelessComponent "SellForm";
  let make ::currencyInput ::receivedInput _children => {
    ...component,
    render: fun _self => <Aux> receivedInput currencyInput </Aux>
  };
};

module WithdrawForm = {
  let component = ReasonReact.statelessComponent "SellForm";
  let make ::currencyInput ::spendInput _children => {
    ...component,
    render: fun _self => <Aux> spendInput currencyInput </Aux>
  };
};

module ExchangeForm = {
  let component = ReasonReact.statelessComponent "SellForm";
  let make ::spendInput ::fromInput ::receivedInput ::toInput _children => {
    ...component,
    render: fun _self =>
      <Aux> spendInput fromInput receivedInput toInput </Aux>
  };
};

type kind =
  | Buy
  | Sell
  | Deposit
  | Withdraw
  | Exchange;

let string_of_kind =
  fun
  | Buy => "buy"
  | Sell => "sell"
  | Deposit => "deposit"
  | Withdraw => "withdraw"
  | Exchange => "exchange";

type state = {
  kind,
  received: string,
  spend: string,
  crypto: string,
  cash: string,
  from: string, /* exchange */
  _to: string, /* exchange */
  timestamp: float
};

type action =
  | ChangeKind kind
  | ChangeReceived string
  | ChangeSpend string
  | ChangeCrypto string
  | ChangeCash string
  | ChangeFrom string
  | ChangeTo string
  | ChangeTimestamp float;

exception UnknownKey;

let changeKind event =>
  ChangeKind (
    switch (valueFromEvent event) {
    | "buy" => Buy
    | "sell" => Sell
    | "deposit" => Deposit
    | "withdraw" => Withdraw
    | _ /* exchange */ => Exchange
    }
  );

let changeReceived event => ChangeReceived (valueFromEvent event);

let changeCrypto event => ChangeCrypto (valueFromEvent event);

let changeSpend event => ChangeSpend (valueFromEvent event);

let changeCash event => ChangeCash (valueFromEvent event);

let changeFrom event => ChangeFrom (valueFromEvent event);

let changeTo event => ChangeTo (valueFromEvent event);

let changeTimestamp timestamp => ChangeTimestamp timestamp;

let handleSubmit onSubmit values _event => onSubmit values;

let component = ReasonReact.reducerComponent "TransactionForm";

let make ::cryptos ::cashes ::onSubmit _children => {
  ...component,
  initialState: fun () => {
    kind: Buy,
    received: "",
    spend: "",
    crypto: "",
    cash: "",
    from: "",
    _to: "",
    timestamp: timestamp ()
  },
  reducer: fun action state =>
    switch action {
    | ChangeKind kind => ReasonReact.Update {...state, kind}
    | ChangeReceived received => ReasonReact.Update {...state, received}
    | ChangeSpend spend => ReasonReact.Update {...state, spend}
    | ChangeCrypto crypto => ReasonReact.Update {...state, crypto}
    | ChangeCash cash => ReasonReact.Update {...state, cash}
    | ChangeFrom from => ReasonReact.Update {...state, from}
    | ChangeTo _to => ReasonReact.Update {...state, _to}
    | ChangeTimestamp timestamp => ReasonReact.Update {...state, timestamp}
    },
  render:
    fun {
          reduce,
          state: {kind, received, spend, crypto, cash, from, _to, timestamp}
        } => {
    let receivedInput =
      <div className="received">
        <Inputs.Number
          value=received
          onChange=(reduce changeReceived)
          placeholder="Received"
        />
      </div>;
    let spendInput =
      <div className="spend">
        <Inputs.Number
          value=spend
          onChange=(reduce changeSpend)
          placeholder="Spend"
        />
      </div>;
    let cryptoInput =
      <div className="crypto">
        <Inputs.Select
          value=crypto
          onChange=(reduce changeCrypto)
          options=(List.map Currency.(fun (id, {name}) => (id, name)) cryptos)
          selectText="Select crypto"
        />
      </div>;
    let cashInput =
      <div className="cash">
        <Inputs.Select
          value=cash
          onChange=(reduce changeCash)
          options=(List.map (fun (id, _) => (id, sup id)) cashes)
          selectText="Select cash"
        />
      </div>;
    let currencyInput =
      <div className="currency">
        <Inputs.CurrencyInput crypto=cryptoInput cash=cashInput />
      </div>;
    let fromInput =
      <div className="from">
        <Inputs.Select
          value=from
          onChange=(reduce changeFrom)
          options=(List.map Currency.(fun (id, {name}) => (id, name)) cryptos)
          selectText="Select crypto"
        />
      </div>;
    let toInput =
      <div className="to">
        <Inputs.Select
          value=_to
          onChange=(reduce changeTo)
          options=(List.map Currency.(fun (id, {name}) => (id, name)) cryptos)
          selectText="Select crypto"
        />
      </div>;
    let handleSubmit _event => {
      open! Transaction;
      open! Currency;
      let cash' () => List.assoc cash cashes;
      let spend' () => fs spend;
      let crypto' () => List.assoc crypto cryptos;
      let received' () => fs received;
      let from' () => List.assoc from cryptos;
      let to' () => List.assoc _to cryptos;
      let currency' () => crypto != "" ? Crypto (crypto' ()) : Cash (cash' ());
      switch kind {
      | Buy =>
        onSubmit (
          Buy (cash' ()) (spend' ()) (crypto' ()) (received' ()),
          timestamp
        )
      | Sell =>
        onSubmit (
          Sell (crypto' ()) (spend' ()) (cash' ()) (received' ()),
          timestamp
        )
      | Deposit => onSubmit (Deposit (currency' ()) (received' ()), timestamp)
      | Withdraw => onSubmit (Withdraw (currency' ()) (spend' ()), timestamp)
      | Exchange =>
        onSubmit (
          Exchange (from' ()) (spend' ()) (to' ()) (received' ()),
          timestamp
        )
      }
    };
    <div className=("transaction-form " ^ string_of_kind kind)>
      <div className="kind">
        <Inputs.Select
          value=(string_of_kind kind)
          onChange=(reduce changeKind)
          options=[
            ("buy", "Buy"),
            ("sell", "Sell"),
            ("deposit", "Deposit"),
            ("withdraw", "Withdraw"),
            ("exchange", "Exchange")
          ]
        />
      </div>
      (
        switch kind {
        | Buy => <BuyForm cashInput cryptoInput receivedInput spendInput />
        | Sell => <SellForm cashInput cryptoInput receivedInput spendInput />
        | Deposit => <DepositForm currencyInput receivedInput />
        | Withdraw => <WithdrawForm currencyInput spendInput />
        | Exchange =>
          <ExchangeForm fromInput receivedInput spendInput toInput />
        }
      )
      <div className="timestamp">
        <Inputs.Date onChange=(reduce changeTimestamp) />
      </div>
      <div className="add">
        <Inputs.ActionButton
          image=(loadResource "./add.png")
          onClick=handleSubmit
        />
      </div>
    </div>
  }
};
