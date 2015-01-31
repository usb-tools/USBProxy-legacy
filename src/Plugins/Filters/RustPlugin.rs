#![crate_type = "dylib"]
extern crate libc;

#[repr(C)]
struct Plugin {
    cfg: *mut libc::c_void,
}

#[repr(C)]
pub struct Packet<'a> {
    endpoint: u8,
    length: u16,
    filter: bool,
    transmit: bool,
    data: &'a mut [u8; 8],
}

#[no_mangle]
pub const c_abi: libc::c_int = 1;

pub unsafe fn filter_packet(mut payload: Packet) {
    let mut buf = payload.data;
    (*buf)[2] = b'r';
    (*buf)[3] = b'u';
    (*buf)[4] = b's';
    (*buf)[5] = b't';
    (*buf)[6] = b'l';
    (*buf)[7] = b'e';
}

#[no_mangle]
pub fn get_plugin(cfg: *const libc::c_void) -> unsafe fn(Packet) {
    return filter_packet;
}
