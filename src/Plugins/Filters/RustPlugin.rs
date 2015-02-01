#![crate_type = "dylib"]
extern crate libc;

#[repr(C)]
struct Plugin {
    cfg: *mut libc::c_void,
}

type Packet = libc::c_void;
pub type Data<'a> = &'a mut [u8; 8];

#[no_mangle]
pub const c_abi: libc::c_int = 1;

pub unsafe fn filter_packet(mut buf: Data) {
    (*buf)[2] = b'r';
    (*buf)[3] = b'u';
    (*buf)[4] = b's';
    (*buf)[5] = b't';
    (*buf)[6] = b'l';
    (*buf)[7] = b'e';
}

#[no_mangle]
pub fn get_plugin(cfg: *const libc::c_void) -> unsafe fn(Data) {
    return filter_packet;
}
