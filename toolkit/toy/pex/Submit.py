from Meta import Sender, Receiver, pipeable


@pipeable
def submit(sender: Sender, receiver: Receiver):
  # this is much simpler than libunifex version because op is managed object.
  # That means op's lifetime does not need to be handled separately by "blocking"
  op = sender.connect(receiver)
  op.start()

