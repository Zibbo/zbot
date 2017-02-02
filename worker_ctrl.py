import threading
import numpy


class Ctrl:
    def __init__(self):
        pass

class Handler:
    def __init__(self, in_queue, out_queue):
        self.in_queue = in_queue
        self.out_queue = out_queue
        self.run = True
        self.ev_wait_dict = {}
        
    def thread_loop(self):
        while self.run:
            item = self.in_queue.get()
            #figure out direction
            if up:
                self.out_queue.put(item)
            else:
                if item_id not in self.ev_wait_dict:
                    self.ev_wait_dict[item_id] = []
                self.ev_wait_dict[item_id].append(item)
                if len(self.ev_wait_dict[item_id]) >= 2:
                    self.out_queue.put(self.ev_wait_dict[item_id])
                    del self.ev_wait_dict[item_id]
        
class Worker:
    def __init__(self, in_queue, out_queue):
        self.in_queue = in_queue
        self.out_queue = out_queue
        self.run = True

    def thread_loop(self):
        while self.run:
            item = self.in_queue.get()
            if len(item) > 1:
                #going down
                #call 


class Sender:
    def __init__(self, in_queue, loopback_queue):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("", 0))
        self.in_queue = in_queue
        self.loopback_queue = loopback_queue
        self.run = True
        
    def thread_loop(self):
        while self.run:
            item = self.in_queue.get()
            #figure out direction
            if up:
                #figure out next unique state(s)
                for us in unique_states:
                    #modify betting path
                    if us == stored_locally:
                        self.loopback_queue.put(item)
                    else:
                        self.s.sendto(item, us.address)
            else:
                #figure out prev unique_state
                if us == stored_locally:
                    self.loopback_queue.put(item)
                else:
                    self.s.sendto(item, us.address)

                        
class Receiver:
    def __init__(self, out_queue, receive_port = 20001):
        self.s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.bind(("", receive_port))
        self.out_queue = out_queue
        self.run = True
        #start thread
        self.receive_thread = threading.Thread(target=self.thread_loop)
        self.receive_thread.start()
        self.receive_thread.run()
        

    def thread_loop(self):
        while self.run:
            data = self.s.recv()
            #process data
            self.out_queue.put(data)

            
