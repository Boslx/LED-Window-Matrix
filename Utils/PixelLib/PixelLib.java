import java.util.LinkedList;
import java.util.List;

public class PixelLib {

    private static final int command = 0; // for 8 bit colors

    public static byte[] getByteArray(Pixel[][] pixels){
        return byteListToArray(getByteList(pixels));
    }


    public static byte[] getByteArrayChannel(int channel, Pixel[] pixels){
        return byteListToArray(getByteListRow(channel,pixels));
    }

    private static List<Byte> getByteListRow(int channel, Pixel[] pixelArray){
        int length = pixelArray.length * 3;
        byte lowByte = (byte)(length & 0xFF);
        byte highByte = (byte)((length >> 8) & 0xFF);
        List<Byte> bytelist = new LinkedList<>();
        bytelist.add((byte)channel);                //channel
        bytelist.add((byte)command);                //command
        bytelist.add(highByte);                     //high-byte
        bytelist.add(lowByte);                      //low-byte
       bytelist.addAll(pixelToByte(pixelArray));
        return bytelist;
    }

    private static List<Byte> getByteList(Pixel[][] pixelArray){
        List<Byte> bytelist = new LinkedList<>();
        for (int i = 0; i < pixelArray.length; i++) {
            bytelist.addAll(getByteListRow(i,pixelArray[i]));
        }
        return bytelist;
    }

    private static List<Byte> pixelToByte(Pixel[] pixels){
        List<Byte> bytelist = new LinkedList<>();
        for (Pixel p: pixels) {                 //data
            bytelist.add((byte)p.r);
            bytelist.add((byte)p.g);
            bytelist.add((byte)p.b);
        }
        return bytelist;
    }

    private static byte[] byteListToArray(List<Byte> byteList){
        byte[] byteArray = new  byte[byteList.size()];
        for (int i = 0; i < byteList.size(); i++) {
            byteArray[i] = byteList.get(i);
        }
        return  byteArray;
    }

}
