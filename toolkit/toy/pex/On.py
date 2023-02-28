from Meta import Scheduler, Sender
from Sequence import sequence
from WithQueryValue import with_query_value


def on(scheduler: Scheduler, sender: Sender):
  return sequence(scheduler.schedule(), with_query_value(sender, "get_scheduler", scheduler))
