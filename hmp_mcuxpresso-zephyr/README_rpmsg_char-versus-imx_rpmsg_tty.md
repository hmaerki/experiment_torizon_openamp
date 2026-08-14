# rpmsg_char versus imx_rpmsg_tty


## Conclusion

The big benefit of imx_rpmsg_tty is that it allows to use from the command line:

```bash
echo "hello" > /dev/ttyRPMSG0
cat /dev/ttyRPMSG0
```

## Prompt

difference between https://github.com/nxp-imx/linux-imx/blob/lf-6.18.y/drivers/rpmsg/imx_rpmsg_tty.c and https://github.com/nxp-imx/linux-imx/blob/lf-6.18.y/drivers/rpmsg/rpmsg_char.c

They are both RPMsg drivers, but they expose the RPMsg channel to Linux userspace in **very different ways**:

* `imx_rpmsg_tty.c` → exposes RPMsg as a **Linux TTY/serial device**, e.g. `/dev/ttyRPMSG0`.
* `rpmsg_char.c` → exposes RPMsg as a **generic character device**, e.g. `/dev/rpmsg0`.

In other words:

> **`imx_rpmsg_tty` = RPMsg adapted to the Linux TTY subsystem**
> **`rpmsg_char` = RPMsg exposed almost directly as a userspace byte/message interface**

These are the exact `lf-6.18.y` versions you linked. 

## 1. The userspace interface is the biggest difference

### `imx_rpmsg_tty.c`

It allocates a Linux `tty_driver` and registers it:

```c
rpmsgtty_driver = tty_alloc_driver(1, TTY_DRIVER_UNNUMBERED_NODE);
...
rpmsgtty_driver->driver_name = "rpmsg_tty";
rpmsgtty_driver->name = kasprintf(..., "ttyRPMSG%d", rpdev->dst);
...
tty_register_driver(cport->rpmsgtty_driver);
```

So userspace gets something like:

```text
/dev/ttyRPMSG0
```

and applications can use ordinary serial/TTY APIs:

```bash
echo "hello" > /dev/ttyRPMSG0
cat /dev/ttyRPMSG0
```

The driver implements the TTY `.open()`, `.close()`, `.write()` and `.write_room()` operations. 

### `rpmsg_char.c`

This instead creates a Linux `cdev`:

```c
cdev_init(&eptdev->cdev, &rpmsg_eptdev_fops);
...
cdev_device_add(&eptdev->cdev, &eptdev->dev);
```

and names the device:

```c
dev_set_name(dev, "rpmsg%d", ret);
```

So you get something along the lines of:

```text
/dev/rpmsg0
```

with ordinary `read()`, `write()`, `poll()` and `ioctl()` operations. 

---

## 2. They bind to different RPMsg channel names

This is particularly important on i.MX.

### `imx_rpmsg_tty`

The driver matches:

```c
static struct rpmsg_device_id rpmsg_driver_tty_id_table[] = {
    { .name = "rpmsg-virtual-tty-channel-1" },
    { .name = "rpmsg-virtual-tty-channel" },
    { .name = "rpmsg-openamp-demo-channel" },
    { },
};
```

So this driver is specifically intended for the **virtual TTY channels**. 

### `rpmsg_char`

The generic char driver matches:

```c
static struct rpmsg_device_id rpmsg_chrdev_id_table[] = {
    { .name = "rpmsg-raw" },
    { .name = "rpmsg_chrdev" },
    { },
};
```

So a remote endpoint advertising `rpmsg-raw` or `rpmsg_chrdev` is handled by `rpmsg_char`. 

This means they normally **don't compete for the same channel**.

---

## 3. RX handling is quite different

This is probably the most important implementation difference.

### TTY driver

When an RPMsg packet arrives:

```c
static int rpmsg_tty_cb(...)
{
    ...
    space = tty_prepare_flip_string(&cport->port, &cbuf, len);
    ...
    memcpy(cbuf, data, len);
    tty_flip_buffer_push(&cport->port);
}
```

The received RPMsg payload is pushed into the **TTY flip buffer**. The normal Linux TTY machinery then delivers it to the application. 

Conceptually:

```text
Remote CPU
    |
    | RPMsg packet
    v
RPMsg core
    |
    v
imx_rpmsg_tty
    |
    v
TTY flip buffer
    |
    v
TTY subsystem
    |
    v
/dev/ttyRPMSG0
    |
    v
application
```

That means things such as TTY line discipline and terminal semantics are potentially involved.

---

### Generic RPMsg char driver

`rpmsg_char` does not go through TTY at all.

Its callback does:

```c
skb = alloc_skb(len, GFP_ATOMIC);
skb_put_data(skb, buf, len);

spin_lock(&eptdev->queue_lock);
skb_queue_tail(&eptdev->queue, skb);
spin_unlock(&eptdev->queue_lock);

wake_up_interruptible(&eptdev->readq);
```

So every received RPMsg payload is put into an **SKB queue**. 

Then userspace `read()` dequeues one SKB:

```c
skb = skb_dequeue(&eptdev->queue);
...
copy_to_iter(skb->data, use, to);
```

Conceptually:

```text
Remote CPU
    |
    | RPMsg packet
    v
RPMsg core
    |
    v
rpmsg_char
    |
    v
SKB queue
    |
    v
/dev/rpmsg0
    |
    v
application
```

This is much closer to a **raw message transport**.

---

## 4. TX is also different

### `imx_rpmsg_tty`

The TTY `.write()` ultimately calls:

```c
rpmsg_send(rpdev->ept, tbuf, ...);
```

and explicitly splits the write into **256-byte chunks**:

```c
#define RPMSG_MAX_SIZE 256
```

If userspace writes 1000 bytes, this driver sends approximately:

```text
256
256
256
232
```

as separate RPMsg messages. 

So:

```text
write(fd, 1000 bytes)
        |
        +--> RPMsg message 256
        +--> RPMsg message 256
        +--> RPMsg message 256
        +--> RPMsg message 232
```

---

### `rpmsg_char`

The generic char driver doesn't impose that 256-byte application-level chunking.

It gets the complete userspace write:

```c
size_t len = iov_iter_count(from);
```

copies it to kernel memory, then does:

```c
rpmsg_sendto(eptdev->ept, kbuf, len, eptdev->chinfo.dst);
```

or, for nonblocking I/O:

```c
rpmsg_trysendto(...)
```

The RPMsg transport/core then deals with its own message-size constraints. 

So the API abstraction is:

```text
TTY:
    write() -> stream-oriented interface

rpmsg_char:
    write() -> RPMsg endpoint/message-oriented interface
```

---

## 5. `rpmsg_char` has endpoint management that the TTY driver doesn't

This is a major advantage of `rpmsg_char`.

It has:

```c
struct rpmsg_eptdev {
    ...
    struct rpmsg_endpoint *ept;
    struct rpmsg_endpoint *default_ept;
    ...
};
```

and on open it can either reuse the default endpoint or create a new endpoint:

```c
if (eptdev->default_ept)
    ept = eptdev->default_ept;
else
    ept = rpmsg_create_ept(...);
```

It also supports endpoint destruction and flow-control IOCTLs. 

The supported IOCTLs include:

```text
RPMSG_GET_OUTGOING_FLOWCONTROL
RPMSG_SET_INCOMING_FLOWCONTROL
RPMSG_DESTROY_EPT_IOCTL
```



The NXP TTY driver has none of this endpoint-management interface.

---

## 6. `poll()` / nonblocking behavior

`rpmsg_char` explicitly supports:

```c
.poll
```

and has proper blocking/nonblocking read behavior:

```c
if (filp->f_flags & O_NONBLOCK)
    return -EAGAIN;
```

and waits on:

```c
wait_event_interruptible(eptdev->readq, ...)
```

It also has `rpmsg_trysendto()` for nonblocking writes. 

The TTY driver relies primarily on the Linux TTY infrastructure for this behavior rather than implementing a raw RPMsg userspace queue itself.

---

## 7. Flow control

`rpmsg_char` in this branch has explicit **RPMsg flow-control support**:

```c
static int rpmsg_ept_flow_cb(...)
```

and tracks:

```c
bool remote_flow_restricted;
bool remote_flow_updated;
```

It exposes that information through IOCTLs. 

The TTY implementation doesn't have corresponding RPMsg flow-control handling.

That's another indication that `rpmsg_char` is intended as the more generic/raw interface.

---

## 8. The practical difference

Think of the two interfaces like this:

|                               | `imx_rpmsg_tty`              | `rpmsg_char`                           |
| ----------------------------- | ---------------------------- | -------------------------------------- |
| Purpose                       | Virtual serial port          | Generic RPMsg endpoint                 |
| Userspace device              | `/dev/ttyRPMSGx`             | `/dev/rpmsgx`                          |
| Linux subsystem               | TTY                          | Character device                       |
| Channel names                 | `rpmsg-virtual-tty-*`, demo  | `rpmsg-raw`, `rpmsg_chrdev`            |
| RX buffering                  | TTY flip buffer              | SKB queue                              |
| TX                            | `rpmsg_send()`               | `rpmsg_sendto()` / `rpmsg_trysendto()` |
| 256-byte application chunking | **Yes**                      | **No explicit 256-byte chunking**      |
| `poll()`                      | Via TTY                      | Explicitly implemented                 |
| Nonblocking TX                | Not explicitly exposed       | Yes                                    |
| RPMsg flow control            | No                           | Yes                                    |
| Endpoint creation             | Uses existing RPMsg endpoint | Can create/reuse endpoint              |
| IOCTL interface               | No RPMsg-specific IOCTL      | Yes                                    |
| Best for                      | Serial-style applications    | Raw RPMsg applications                 |

The underlying RPMsg transport is still the same. The difference is **how the kernel presents that transport to userspace**.

---

# A subtle but important point

`rpmsg_char` is not simply "another TTY driver."

The TTY driver converts RPMsg into a **byte-stream/terminal abstraction**:

```text
             RPMsg
               |
               v
       +---------------+
       | imx_rpmsg_tty |
       +---------------+
               |
               v
        Linux TTY layer
               |
               v
        /dev/ttyRPMSG0
```

Whereas `rpmsg_char` is much closer to:

```text
             RPMsg
               |
               v
        +-------------+
        | rpmsg_char  |
        +-------------+
               |
               v
          /dev/rpmsg0
```

The latter preserves much more of the RPMsg endpoint semantics.

---

## One particularly important consequence

Suppose your remote Cortex-M firmware does:

```text
send("ABC")
send("DEF")
```

With `rpmsg_char`, userspace's `read()` sees data coming through the RPMsg message queue, so **message boundaries are much more naturally preserved**.

With the TTY interface, you're putting those bytes into the TTY subsystem. The application is dealing with a **TTY byte stream**, not a raw RPMsg-message API.

So if you're implementing a protocol such as:

```text
[header][length][payload]
```

over RPMsg, I'd generally prefer **`rpmsg_char`**.

If you want the remote processor to look like a serial port to existing software, I'd use **`imx_rpmsg_tty`**.

---

## One more i.MX-specific detail

The NXP TTY driver also sends:

```c
#define MSG "hello world!"
...
rpmsg_send(rpdev->ept, MSG, strlen(MSG));
```

during probe, with the explicit comment that this tells the remote processor about the channel. 

That's **NXP/i.MX-specific behavior** rather than a fundamental requirement of RPMsg.

The generic `rpmsg_char` probe doesn't send such a greeting; it creates the character-device representation of the already-discovered RPMsg channel. 

---

### Bottom line

If you're deciding **which one to use for a new i.MX RPMsg application**:

**Use `imx_rpmsg_tty` when:**

```text
Remote MCU <---- RPMsg ----> Linux
                              |
                              +-- /dev/ttyRPMSG0
                                  |
                                  +-- serial/TTY application
```

**Use `rpmsg_char` when:**

```text
Remote MCU <---- RPMsg ----> Linux
                              |
                              +-- /dev/rpmsg0
                                  |
                                  +-- custom RPMsg protocol/application
```

And if your real question is **"why does my firmware work with `rpmsg_tty` but not `rpmsg_char` (or vice versa)?"**, the next thing to examine is the **remote endpoint/channel name and the RPMsg device-tree/remoteproc setup**. The two drivers intentionally match different RPMsg service names. 

[imx_rpmsg_tty.c](https://github.com/nxp-imx/linux-imx/blob/lf-6.18.y/drivers/rpmsg/imx_rpmsg_tty.c?utm_source=chatgpt.com) · [rpmsg_char.c](https://github.com/nxp-imx/linux-imx/blob/lf-6.18.y/drivers/rpmsg/rpmsg_char.c?utm_source=chatgpt.com) 
