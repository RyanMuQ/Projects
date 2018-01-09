from quantopian.algorithm import attach_pipeline, pipeline_output
from quantopian.pipeline import Pipeline
from quantopian.pipeline.data.builtin import USEquityPricing
from quantopian.pipeline.factors.morningstar import MarketCap
from quantopian.pipeline.factors import AverageDollarVolume
from quantopian.pipeline.factors import AnnualizedVolatility
import numpy as np


# transpose a float number round to integer
def round2int(x):
    x = round(x)
    if x < 0:
        return -int(-x + 0.01)
    else:
        return int(x + 0.01)

class Portfolio:
    # Initialize the portfolio
    def __init__(self, initial_value, name, context, data):
        self.position = {}
        for stock in context.security_set: # set all positions 0
            self.position[stock] = 0.0
        self.value = initial_value         # total money
        self.cash = initial_value          # amount of cash
        self.context = context             # copy the context object
        self.data = data                   # copy the data object
        self.name = name                   # give it a name
    
    # combine two portfolios and calculate the combined positions
    def combine(self, ports, weights):
        ret = Portfolio(0, "combine", self.context, self.data)
        for i in range(len(ports)):
            port = ports[i]
            ret.cash += weights[i] * port.cash
            ret.name += "_" + port.name
            for stock in port.position:
                ret.position[stock] += weights[i] * port.position[stock]
        ret.refresh()
        return ret
    
    # updating the current total value
    def refresh(self):
        self.value = self.cash
        for stock, amount in self.position.iteritems():
            self.value += self.data.current(stock, 'price') * amount
        
    # assign a fixed weights to this portfolio and calculate positions
    def set_percent(self, weights):
        self.sell_out()
        for stock, weight in weights.iteritems():
            price = self.data.current(stock, 'price')
            self.position[stock] = weight * self.value / price
            self.cash -= price * self.position[stock]
    
    # void the position and hold all the money as cash
    def sell_out(self):
        self.refresh()
        self.cash = self.value
        for stock in self.position:
            self.position[stock] = 0
    
    # output the current positions held in this portfolio
    def output(self):
        self.refresh()
        print "Portfolio name: {0}".format(self.name)
        print "Total value: {0}\tCash: {1}".format(self.value, self.cash)
        for stock, amount in self.position.iteritems():
            if amount != 0:
                print "{0}\t{1}\t{2}".format(stock.symbol,
                                             amount,
                                             self.data.current(stock, 'price'))
        print ""
        
    
# Sort the dataframe by dollar_volume factor
def data_ranking(df, key):
    
    # sort by dollar_volume factor
    df_ranked = df.sort_values(by=key, ascending=False)
    
    return df_ranked

  
def make_pipeline():
    
    # Using average dollar volume as the factor measuring liqiudity
    return Pipeline(columns={
            'market_cap': MarketCap(),
            'volatility': AnnualizedVolatility(window_length=21),
            'dollar_volume': AverageDollarVolume(window_length=21)
           })

################################################################

def initialize(context):
    # Trading frequency for the high-frequency portfolio
    context.frequency = 2
    
    # Amount of hedged portfolio to be held
    context.multplier = 1.0

    # Low return threshold to stop trading in this day.
    context.stop_threshold = -3e-4
    
    # Set the transaction fee
    set_commission(commission.PerShare(cost=0.0, min_trade_cost=0.0))
    #set_slippage(slippage.VolumeShareSlippage(volume_limit=1, price_impact=0))
    
    # Record the positions in buy & hold portfolio at the beginning of the day
    schedule_function(rebalance_start,
                      date_rules.every_day(),
                      time_rules.market_open(minutes=1))
    
    # Rebalance every {frequency} minutes every trading day.
    for i in np.arange(context.frequency, 390, context.frequency):
        schedule_function(rebalance,
                          date_rules.every_day(),
                          time_rules.market_open(minutes=i))
    
    # Monitor to stop trading
    for i in np.arange(context.frequency + 10, 390, 1):
        schedule_function(stop_trading_monitor,
                          date_rules.every_day(),
                          time_rules.market_open(minutes=i))

    # Clean the positions at the end of the day.
    schedule_function(clean_positions,
                      date_rules.every_day(),
                      time_rules.market_close(minutes=1))
    
    # Record tracking variables at the end of each day.
    schedule_function(record_vars,
                      date_rules.every_day(),
                      time_rules.market_close())
     
    # Create our dynamic stock selector.
    attach_pipeline(make_pipeline(), 'Data')

 
def before_trading_start(context, data):

    # Switch to stop trading !!!
    context.stop_trading = False # !!!
    
    # apply the logic to the data pull in order to get a ranked list of equities
    context.output = pipeline_output('Data')
    context.output = data_ranking(context.output, 'dollar_volume').head(500)
    context.output = data_ranking(context.output, 'volatility').head(100)
    context.output = data_ranking(context.output, 'market_cap').head(50)
    
    # create lists of stocks with high liquidity
    context.security_set = set(context.output.index)
    
    # initialize the two sub-portfolios
    context.portfolio_high = Portfolio(context.multplier * context.account.settled_cash,
                                       "{0}-min frequency".format(context.frequency),
                                       context, data)
    context.portfolio_low = Portfolio(context.multplier * context.account.settled_cash,
                                      "390-min frequency",
                                      context, data)
    
    # get ready to record the minute data
    context.low_prices = []
    context.hedge_prices = []
  
     
# weight is propotional to market_cap^p; default: p=0 (equal weight)
def assign_weights(context, data, p=0.0):
    
    # take the secutiry list which is made of securities with top 50 high market cap
    securities = list(context.security_set)
    
    market_caps = context.output.loc[securities,'market_cap']
    market_caps = market_caps.fillna(1)
    
    weight = market_caps ** p
    
    weight_sum = weight.sum()
    weight = weight / weight_sum
    
    return weight
 
    
# do the everytime-rebalance
def rebalance(context, data):
    # assign weights to securities in portfolio
    weights = assign_weights(context, data, p=0.0)
    
    # refresh all portfolios
    context.portfolio_high.refresh()
    context.portfolio_low.refresh()
    
    # rebalance the high-frequency portfolio
    context.portfolio_high.set_percent(weights)
    
    # calculate the combination of two portfolios
    portfolio_trade = context.portfolio_high.combine(
        [context.portfolio_high, context.portfolio_low],
        [1.0, -1.0])

    # close the open orders
    close_open_orders(context, data)

    if not context.stop_trading: # !!!
        # make the position to the target amount
        for stock,amount in portfolio_trade.position.iteritems():
            if data.can_trade(stock):
                order_target(stock, round2int(amount))
    
    # clean the needless assets
    everytime_clean(context, data)

    
# Monitor run every minute to check if stopping trading
def stop_trading_monitor(context, data):
    
    # trade if the stop-trading switch is closed.
    if not context.stop_trading:

        if context.hedge_prices[-1] / context.low_prices[-1] < context.stop_threshold:
            
            # clean position
            close_open_orders(context, data)
            clean_positions(context, data)
            
            # close trading
            context.stop_trading = True

            
# rebalance at the begin of the trading day. No actual trading since those two portfolios are prefectly hedged.
def rebalance_start(context, data):
    # equal weights
    weights = assign_weights(context, data, p=0.0)
    
    # set up two portfolios
    context.portfolio_high.set_percent(weights)
    context.portfolio_low.set_percent(weights)
    

# clean all open orders
def close_open_orders(context, data):

    open_orders = get_open_orders()
    for security, orders in open_orders.iteritems():
        for order in orders:
            log.warn("[Open order canceled] {0} shares of {1} is canceled.".format(order.amount - order.filled, security))
            cancel_order(order)

            
# make sure all untradeable securities are sold off each time
def everytime_clean(context, data):
    
    for stock in context.portfolio.positions:
        if stock not in context.security_set and data.can_trade(stock):
            order_target_percent(stock, 0)

            
# Clean the positions at the end of the day. 
def clean_positions(context, data):
    
    for stock in context.portfolio.positions:
        if data.can_trade(stock):
            order_target_percent(stock, 0)
    
    returns = [hedge / low for hedge, low in zip(context.hedge_prices, context.low_prices)]
    log.info("Lowest return: {0:.2f}%%\tDay return: {1:.2f}%%.".format(np.min(returns) * 10000, returns[-1] * 10000))
            
 
def record_vars(context, data):
    
    log_prices = np.log(np.array(context.low_prices))
    record(volatility = 100 * np.std(log_prices[1:] - log_prices[:-1]))
    
 
# track the prices of the high-frequency portfolio
def handle_data(context,data):
    
    context.portfolio_low.refresh()
    low_value = context.portfolio_low.value

    context.portfolio_high.refresh()
    hedge_value = context.portfolio_high.value - context.portfolio_low.value
    
    context.low_prices.append(low_value)
    context.hedge_prices.append(hedge_value)