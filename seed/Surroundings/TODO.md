1. There are still these sounds artifacts. They appear endlessly with equal time intervals. I noticed changing the buffersize changes this behavior (as you mentioned as well). With a buffersize of 4096 the artifacts appear about 50x per second (just a guess), and with a buffersize of 131072 just once a second. Still in a fixed rhythm. Even when I output 0.0f on the audio callback, the artifacts are still there. Of course it might have something to do with my breadboarding situation. I did notice that sometimes after a while it stops.

2. When I play the storm example with the updated WavPlayer, I definitely hear some "bitcrushing" going on in the rumbling of the thunder. Increasing the audio does seem to reduce the amount of bitcrushing heard, or at least alter the sound of it. No clue what audio block size I should use though...

3. I need my audio files to form a perfect loop. i'm confident my audio files are bounced in a perfect loop, but every time the WavPlayer loops my audio file, I hear an audio freeze of (I guess) 50-100ms, until it repeats. I think it may have something to do with how the audio buffer end is reached, and it should buffer from the start seamlessly.

4. After a while of playback, the program seems to hang in a one-second (or so) loop sometimes.

5. LED flickering (PWM)
