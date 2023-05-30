#include<HTTPClient.h>
//#include <Arduino.h>
//#include <esp32-hal-log.h>  
//
//#ifdef HTTPCLIENT_1_1_COMPATIBLE
//#include <WiFi.h>
//#include <WiFiClientSecure.h>
//#endif
//
#include <StreamString.h>
//#include <base64.h>
class HTTPClient_Custom :public HTTPClient {
public:
    int writeToStream_mag(Stream* stream)
    {
        if (!stream) {
            return returnError(HTTPC_ERROR_NO_STREAM);
        }

        if (!connected()) {
            return returnError(HTTPC_ERROR_NOT_CONNECTED);
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = _size;
        int ret = 0;

        if (_transferEncoding == HTTPC_TE_IDENTITY) {
            //if (1) {
            ret = writeToStreamDataBlock_mag(stream, len);

            // have we an error?
            if (ret < 0) {
                return returnError(ret);
            }
        }
        else if (_transferEncoding == HTTPC_TE_CHUNKED) {
            int size = 0;
            while (1) {
                if (!connected()) {
                    return returnError(HTTPC_ERROR_CONNECTION_LOST);
                }
                String chunkHeader = _client->readStringUntil('\n');

                if (chunkHeader.length() <= 0) {
                    return returnError(HTTPC_ERROR_READ_TIMEOUT);
                }

                chunkHeader.trim(); // remove \r

                // read size of chunk
                len = (uint32_t)strtol((const char*)chunkHeader.c_str(), NULL, 16);
                size += len;
                log_d(" read chunk len: %d", len);

                // data left?
                if (len > 0) {
                    int r = writeToStreamDataBlock_mag(stream, len);
                    if (r < 0) {
                        // error in writeToStreamDataBlock
                        return returnError(r);
                    }
                    ret += r;
                }
                else {

                    // if no length Header use global chunk size
                    if (_size <= 0) {
                        _size = size;
                    }

                    // check if we have write all data out
                    if (ret != _size) {
                        return returnError(HTTPC_ERROR_STREAM_WRITE);
                    }
                    break;
                }

                // read trailing \r\n at the end of the chunk
                char buf[2];
                auto trailing_seq_len = _client->readBytes((uint8_t*)buf, 2);
                if (trailing_seq_len != 2 || buf[0] != '\r' || buf[1] != '\n') {
                    return returnError(HTTPC_ERROR_READ_TIMEOUT);
                }

                delay(0);
            }
        }
        else {
            return returnError(HTTPC_ERROR_ENCODING);
        }

        //    end();
        disconnect(true);
        return ret;
    }

    void cacheWriteSerial(void)
    {
        // _size can be -1 when Server sends no Content-Length header
        if (_size > 0 || _size == -1) {
            StreamString sstring;
            writeToStream_mag(&sstring);
        }
    }

    int writeToStreamDataBlock_mag(Stream* stream, int size)
    {
        int buff_size = HTTP_TCP_BUFFER_SIZE;
        int len = size;
        int bytesWritten = 0;

        // if possible create smaller buffer then HTTP_TCP_BUFFER_SIZE
        if ((len > 0) && (len < HTTP_TCP_BUFFER_SIZE)) {
            buff_size = len;
        }

        // create buffer for read
        uint8_t* buff = (uint8_t*)malloc(buff_size);

        if (buff) {
            // read all data from server
            while (connected() && (len > 0 || len == -1)) {

                // get available data size
                size_t sizeAvailable = _client->available();

                if (sizeAvailable) {

                    int readBytes = sizeAvailable;

                    // read only the asked bytes
                    if (len > 0 && readBytes > len) {
                        readBytes = len;
                    }

                    // not read more the buffer can handle
                    if (readBytes > buff_size) {
                        readBytes = buff_size;
                    }

                    // stop if no more reading    
                    if (readBytes == 0)
                        break;

                    // read data
                    int bytesRead = _client->readBytes(buff, readBytes);

                    // write it to Stream
                    //int bytesWrite = stream->write(buff, bytesRead);
                    int bytesWrite = bytesRead;
                    bytesWritten += bytesWrite;

                    // are all Bytes a writen to stream ?
                    if (bytesWrite != bytesRead) {
                        log_d("short write asked for %d but got %d retry...", bytesRead, bytesWrite);

                        // check for write error
                        if (stream->getWriteError()) {
                            log_d("stream write error %d", stream->getWriteError());

                            //reset write error for retry
                            stream->clearWriteError();
                        }

                        // some time for the stream
                        delay(1);

                        int leftBytes = (readBytes - bytesWrite);

                        // retry to send the missed bytes
                        bytesWrite = stream->write((buff + bytesWrite), leftBytes);
                        bytesWritten += bytesWrite;

                        if (bytesWrite != leftBytes) {
                            // failed again
                            log_w("short write asked for %d but got %d failed.", leftBytes, bytesWrite);
                            free(buff);
                            return HTTPC_ERROR_STREAM_WRITE;
                        }
                    }

                    // check for write error
                    if (stream->getWriteError()) {
                        log_w("stream write error %d", stream->getWriteError());
                        free(buff);
                        return HTTPC_ERROR_STREAM_WRITE;
                    }

                    // count bytes to read left
                    if (len > 0) {
                        len -= readBytes;
                    }
                    for (int i = 0; i < readBytes; i++)
                    {
                        while (Serial.availableForWrite() < 10) {
                        }
                        Serial.write(buff[i]);
                    }
                    stream->flush(); // 刷新输出
                    StreamString nonObject; // 创建一个临时的 StreamString 对象
                    stream = &nonObject; // 重定向指针
                    delay(0);
                }
                else {
                    delay(1);
                }
            }


            free(buff);

            log_d("connection closed or file end (written: %d).", bytesWritten);

            if ((size > 0) && (size != bytesWritten)) {
                log_d("bytesWritten %d and size %d mismatch!.", bytesWritten, size);
                return HTTPC_ERROR_STREAM_WRITE;
            }

        }
        else {
            log_w("too less ram! need %d", HTTP_TCP_BUFFER_SIZE);
            return HTTPC_ERROR_TOO_LESS_RAM;
        }

        return bytesWritten;
    }
};