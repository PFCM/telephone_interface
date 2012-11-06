Telephone t;
t.init(0);
PulseOsc n => BPF b => DelayA d => JCRev rev => dac;
PulseOsc o => b;
Noise noise => HPF noisef => b;
d => Gain fb => b;
0.2 => fb.gain;
400 => int f;
5 => float r;
f => b.freq;
r => b.Q;
0.01 => rev.mix;
1::second => d.max => d.delay;

spork ~accel();

fun void accel() {
    while (true)
    {
        t.accelEvent => now;
        if (t.accelEvent.which == 0)
            t.accels[0]*10 => f;
        else if (t.accelEvent.which == 1)
            t.accels[1]*0.01 => r;
        r => b.Q;
        f => b.freq;
        
        <<<t.accels[0] + "  " + t.accels[1]>>>;
    }
}

while (true) {
 
        t.pots[0]/127.0 => rev.mix;
        t.pots[2]/127.0 => fb.gain;
        440 * (t.accels[0]/127.0*2-1) => n.freq;
        330 * (t.accels[1]/127.0*2-1) => o.freq;
        t.blue/127.0 => n.width => o.width;
        (t.red/127.0)::second => d.delay;
        t.green*10 => noisef.freq;
        t.toggle / 127.0 * 0.8 => noise.gain;
        t.pots[1] /127.0*2 => rev.gain;
       
        10::ms => now;   
}