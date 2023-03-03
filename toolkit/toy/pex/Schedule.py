import typing
from Meta import Scheduler, Sender, Receiver, OperationState


class LazySchedulerSender(Sender):
  def connect(self, r: Receiver) -> OperationState:
    scheduler = r.get_scheduler()
    return scheduler.schedule().connect(r)


def schedule(scheduler: typing.Optional[Scheduler] = None):
  if scheduler is None:
    return LazySchedulerSender()
  else:
    return scheduler.schedule()


class CurrentScheduler(Scheduler):
  def schedule(self) -> Sender:
    return schedule()


current_scheduler = CurrentScheduler()
