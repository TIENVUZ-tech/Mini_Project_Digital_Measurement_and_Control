"""
protocol.py
Greenhouse Communication Protocol

Frame format:

SOF(1) | TYPE(1) | ID(1) | LEN_H(1) | LEN_L(1) | PAYLOAD(0~64)

ID = (GID << 4) | MID
"""

import struct

# ============================================================
# FRAME CONSTANTS
# ============================================================

FRAME_SOF = 0xAA

TYPE_CMD = 0x0F
TYPE_RSP = 0x1F

FRAME_HEADER_SIZE = 5
FRAME_MAX_PAYLOAD = 64


# ============================================================
# GROUP ID
# ============================================================

GID_SYSTEM = 0x0
GID_SENSOR = 0x1
GID_ACTUATOR = 0x2
GID_PID = 0x3
GID_ERROR = 0x4
GID_COMM = 0xF


# ============================================================
# MESSAGE ID
# ============================================================

MID_SET_MODE = 0x1
MID_STATUS_REPORT = 0x2
MID_SET_SETPOINT = 0x3
MID_SET_REPORT_MODE = 0x4

MID_SENSOR_REPORT = 0x1

MID_MANUAL_ACTUATOR = 0x1

MID_SET_PID = 0x1

MID_ECHO = 0x1


# ============================================================
# RESPONSE STATUS
# ============================================================

RSP_OK = 0x00
RSP_FAIL = 0xFF

RSP_SET_MODE_OK = 0x01
RSP_SET_MODE_FAIL = 0x02
RSP_STATUS_REPORT_OK = 0x03
RSP_STATUS_REPORT_FAIL = 0x04
RSP_SET_SETPOINT_OK = 0x05
RSP_SET_SETPOINT_FAIL = 0x06
RSP_SET_REPORT_MODE_OK = 0x07
RSP_SET_REPORT_MODE_FAIL = 0x08
RSP_SYSTEM_FAIL = 0x0F

RSP_MANUAL_ACTUATOR_OK = 0x01
RSP_MANUAL_ACTUATOR_FAIL = 0x02
RSP_ACTUATOR_FAIL = 0x0F

RSP_SET_PID_OK = 0x01
RSP_SET_PID_FAIL = 0x02
RSP_PID_FAIL = 0x0F

RSP_INVALID_LENGTH = 0x01
RSP_GID_INVALID = 0x02


# ============================================================
# MODE
# ============================================================

MODE_AUTO = 0x00
MODE_MANUAL = 0x01


# ============================================================
# REPORT MODE
# ============================================================

REPORT_ONE_SHOT = 0x00
REPORT_STREAM = 0x01


# ============================================================
# ACTUATOR
# ============================================================

ACTUATOR_FAN = 0x00
ACTUATOR_HEATER = 0x01

ACT_OFF = 0x00
ACT_ON = 0x01


# ============================================================
# ID HELPERS
# ============================================================

def make_id(gid: int, mid: int) -> int:
    return ((gid & 0x0F) << 4) | (mid & 0x0F)


def split_id(id_byte: int):
    return (
        (id_byte >> 4) & 0x0F,
        id_byte & 0x0F
    )


# ============================================================
# FRAME BUILDER
# ============================================================

def build_frame(
    gid: int,
    mid: int,
    payload: bytes = b""
) -> bytes:

    length = len(payload)

    header = bytes([
        FRAME_SOF,
        TYPE_CMD,
        make_id(gid, mid),
        (length >> 8) & 0xFF,
        length & 0xFF
    ])

    return header + payload


# ============================================================
# SYSTEM COMMANDS
# ============================================================

def build_set_mode(mode: int):

    return build_frame(
        GID_SYSTEM,
        MID_SET_MODE,
        bytes([mode])
    )


def build_set_setpoint(setpoint: float):

    payload = struct.pack("<f", setpoint)

    return build_frame(
        GID_SYSTEM,
        MID_SET_SETPOINT,
        payload
    )


def build_set_report_mode(report_mode: int):

    return build_frame(
        GID_SYSTEM,
        MID_SET_REPORT_MODE,
        bytes([report_mode])
    )


def build_request_status():

    return build_frame(
        GID_SYSTEM,
        MID_STATUS_REPORT,
        b""
    )


# ============================================================
# ACTUATOR COMMANDS
# ============================================================

def build_manual_actuator(
    actuator: int,
    state: int
):

    payload = bytes([
        actuator,
        state
    ])

    return build_frame(
        GID_ACTUATOR,
        MID_MANUAL_ACTUATOR,
        payload
    )


# ============================================================
# PID COMMANDS
# ============================================================

def build_set_pid(
    kp: float,
    ki: float,
    kd: float
):

    payload = struct.pack(
        "<fff",
        kp,
        ki,
        kd
    )

    return build_frame(
        GID_PID,
        MID_SET_PID,
        payload
    )


# ============================================================
# DEBUG COMMANDS
# ============================================================

def build_echo(payload: bytes):

    return build_frame(
        GID_COMM,
        MID_ECHO,
        payload
    )


# ============================================================
# FRAME PARSER
# ============================================================

def parse_frame(data: bytes):

    if len(data) < FRAME_HEADER_SIZE:
        return None, 0

    if data[0] != FRAME_SOF:
        return None, 0

    frame_type = data[1]

    if frame_type != TYPE_RSP:
        return None, 0

    id_byte = data[2]

    gid, mid = split_id(id_byte)

    length = (
        (data[3] << 8)
        | data[4]
    )

    if length > FRAME_MAX_PAYLOAD:
        return None, 0

    total_len = FRAME_HEADER_SIZE + length

    if len(data) < total_len:
        return None, 0

    payload = data[
        FRAME_HEADER_SIZE:
        total_len
    ]

    frame = {
        "type": frame_type,
        "gid": gid,
        "mid": mid,
        "id": id_byte,
        "length": length,
        "payload": payload
    }

    return frame, total_len


# ============================================================
# DECODERS
# ============================================================

def decode_status_response(payload):

    if len(payload) < 1:
        return None

    return payload[0]


def decode_sensor_report(payload):

    if len(payload) < 8:
        return None

    temperature, setpoint = struct.unpack(
        "<ff",
        payload[:8]
    )

    return {
        "temperature": temperature,
        "temp_setpoint": setpoint
    }


def decode_status_report(payload):

    if len(payload) < 10:
        return None

    temperature, setpoint = struct.unpack(
        "<ff",
        payload[:8]
    )

    return {
        "temperature": temperature,
        "temp_setpoint": setpoint,
        "fan_state": payload[8],
        "heater_state": payload[9],
        "fan_pwm": (
            (payload[10] << 8) | payload[11]
            if len(payload) >= 12
            else None
        )
    }


def decode_error_report(payload):

    if len(payload) < 3:
        return None

    return {
        "error_code": payload[0],
        "actuator_index": payload[1],
        "error_mid": payload[2]
    }


# ============================================================
# DISPLAY NAMES
# ============================================================

_GID_NAMES = {
    GID_SYSTEM: "SYSTEM",
    GID_SENSOR: "SENSOR",
    GID_ACTUATOR: "ACTUATOR",
    GID_PID: "PID",
    GID_ERROR: "ERROR",
    GID_COMM: "COMM"
}

_MID_NAMES = {
    (GID_SYSTEM, MID_SET_MODE): "SET_MODE",
    (GID_SYSTEM, MID_STATUS_REPORT): "STATUS_REPORT",
    (GID_SYSTEM, MID_SET_SETPOINT): "SET_SETPOINT",
    (GID_SYSTEM, MID_SET_REPORT_MODE): "SET_REPORT_MODE",

    (GID_SENSOR, MID_SENSOR_REPORT): "SENSOR_REPORT",

    (GID_ACTUATOR, MID_MANUAL_ACTUATOR): "MANUAL_ACTUATOR",

    (GID_PID, MID_SET_PID): "SET_PID",

    (GID_ERROR, 0x1): "ACT_ERR_INDEX",
    (GID_ERROR, 0x2): "ACT_ERR_MODE",
    (GID_ERROR, 0x3): "ACT_ERR_UNIT",
    (GID_ERROR, 0x4): "ACT_ERR_VALUE",

    (GID_COMM, MID_ECHO): "ECHO"
}

_STATUS_NAMES = {
    (GID_SYSTEM, MID_SET_MODE, RSP_SET_MODE_OK): "Mode updated",
    (GID_SYSTEM, MID_SET_MODE, RSP_SET_MODE_FAIL): "Mode rejected",
    (GID_SYSTEM, MID_STATUS_REPORT, RSP_STATUS_REPORT_OK): "Status report OK",
    (GID_SYSTEM, MID_STATUS_REPORT, RSP_STATUS_REPORT_FAIL): "Status report failed",
    (GID_SYSTEM, MID_SET_SETPOINT, RSP_SET_SETPOINT_OK): "Setpoint updated",
    (GID_SYSTEM, MID_SET_SETPOINT, RSP_SET_SETPOINT_FAIL): "Setpoint rejected",
    (GID_SYSTEM, MID_SET_REPORT_MODE, RSP_SET_REPORT_MODE_OK): "Report mode updated",
    (GID_SYSTEM, MID_SET_REPORT_MODE, RSP_SET_REPORT_MODE_FAIL): "Report mode rejected",
    (GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_OK): "Actuator updated",
    (GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_FAIL): "Actuator rejected",
    (GID_PID, MID_SET_PID, RSP_SET_PID_OK): "PID updated",
    (GID_PID, MID_SET_PID, RSP_SET_PID_FAIL): "PID rejected",
    (GID_ERROR, 0x0, RSP_INVALID_LENGTH): "Invalid frame length",
    (GID_ERROR, 0x0, RSP_GID_INVALID): "Invalid group ID"
}


def frame_name(gid, mid):

    gid_name = _GID_NAMES.get(
        gid,
        f"GID_0x{gid:X}"
    )

    mid_name = _MID_NAMES.get(
        (gid, mid),
        f"MID_0x{mid:X}"
    )

    return f"{gid_name}.{mid_name}"


def status_name(gid, mid, status):

    exact = _STATUS_NAMES.get(
        (gid, mid, status)
    )

    if exact:
        return exact

    if status == RSP_SYSTEM_FAIL:
        return "Command failed"

    return f"Status 0x{status:02X}"
