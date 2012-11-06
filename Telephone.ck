public class Telephone 
{
    int pots[3];
    int accels[2];
    int lastAccels[2];
    int red, green, blue;
    int toggle;
    
    
    class ButtonEvent extends Event 
    {
        int which;
        int state;
    }
    class AccelEvent extends Event
    {
        int which;
        int delta;
    }
    ButtonEvent button;
    AccelEvent accelEvent;
    
    MidiIn min;
    MidiMsg msg;
    
    public void init(int port) 
    {
        if (!min.open(port))
        {
            <<<"Telephone: could not open MIDI port ", port>>>;
            me.exit();
        }
        spork ~poller();
    }
    
    public void poller()
    {
        while (true) 
        {
            min => now;
            while (min.recv(msg))
            {
                if (msg.data1 == 176) 
                { // control change channel 1
                    if (msg.data2 == 1)
                    {
                        msg.data3 => blue;
                    }
                    else if (msg.data2 == 2)
                    {
                        msg.data3 => red;
                    }
                    else if (msg.data2 == 3)
                    {
                        msg.data2 => green;
                    }
                    else if (msg.data2 > 4 && msg.data2 < 8)
                    {
                        msg.data3 => pots[msg.data2-5];
                    }
                    else if (msg.data2 > 7)
                    {
                        msg.data3 => accels[msg.data2-8];
                            msg.data2-8 => accelEvent.which;
                            accels[msg.data2-8]-lastAccels[msg.data2-8] => accelEvent.delta;
                            accelEvent.broadcast();
                        accels[msg.data2-8] => lastAccels[msg.data2-8];
                    }
                }
                else if (msg.data1 == 144)
                {//note on channel one
                    msg.data2 => button.which;
                    msg.data3 => button.state;
                    if (msg.data2 == 2) msg.data3 => toggle;
                    button.broadcast();
                }
            }
        }
    }
}