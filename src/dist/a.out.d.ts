export interface ESEmu {
  clock(): void;
  powerOn(): void;
  reset(): void;
  clearFrameComplete(): void;
  getRunEmulation(): boolean;
  setRunEmulation(_0: boolean): void;
  getFrameComplete(): boolean;
  setPC(_0: number): void;
  setSampleFrequency(_0: number): void;
  loadROM(_0: number): boolean;
  emulateSample(): number;
  keyDown(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): boolean;
  keyUp(_0: ArrayBuffer|Uint8Array|Uint8ClampedArray|Int8Array|string): boolean;
  getFrameBuffer(): any;
  delete(): void;
}

export interface ByteArr {
  push_back(_0: number): void;
  resize(_0: number, _1: number): void;
  size(): number;
  set(_0: number, _1: number): boolean;
  get(_0: number): any;
  delete(): void;
}

export interface MainModule {
  ESEmu: {new(): ESEmu};
  ByteArr: {new(): ByteArr};
}
