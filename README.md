# MediaFoundationTransform

## MFTAsynchronous

###  Common

Some helper files used by differents projects.

### MFTAsynchronousAudio

* MFTAsynchronous show minimal implementation for an asynchronous audio transform.
* This MFT works with media session pipeline.
* Does not show dynamic format change code handling.
* Check shutdown status, even if documentation says :
> The client must not use the MFT after calling Shutdown.
* Does not handle Unlocking (MF_TRANSFORM_ASYNC_UNLOCK). This MFT doesn't care about applications who do not use this data processing model.
* This MFT just handles one input sample, and one output sample. There is no real sense to do this for an asynchronous MFT, it is just to show the minimal implementation (i will do it later).
* For now, just one audio file format can be use : 2 channels, 16 bits per sample, 48000 Hz.
* Markers : this MFT does not place markers after output sample.

### MFTAsynchronousVideo

In progress.

### MFTAsynchronousPlayer

In progress.
