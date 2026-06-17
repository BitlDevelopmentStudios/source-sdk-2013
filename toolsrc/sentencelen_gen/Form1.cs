using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using ValveKeyValue;

namespace sentencelen_gen
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        //https://stackoverflow.com/questions/2743068/determine-if-string-is-newline-in-c-sharp
        static bool StringIsNewLine(string s)
        {
            return (!string.IsNullOrEmpty(s)) &&
                (!string.IsNullOrWhiteSpace(s)) &&
                (((s.Length == 1) && (s[0] == 8203)) ||
                ((s.Length == 2) && (s[0] == 8203) && (s[1] == 8203)));
        }

        private void importSentences_Click(object sender, EventArgs e)
        {
            OpenFileDialog loadDialog = new OpenFileDialog();
            loadDialog.FileName = "sentences.txt";
            loadDialog.Filter = "Sentence Text Files | sentences.txt";
            loadDialog.DefaultExt = "txt";
            DialogResult result = loadDialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                sentenceList.Rows.Clear();

                string fileName = loadDialog.FileName;

                // Open the text file using a stream reader.
                using (StreamReader reader = new(fileName))
                {
                    string? line;
                    while ((line = reader.ReadLine()) != null)
                    {
                        // remove whitespace
                        if (string.IsNullOrWhiteSpace(line))
                            continue;

                        // remove newlines
                        if (StringIsNewLine(line))
                            continue;

                        // no comments
                        if (line.Contains("//"))
                            continue;

                        // no virtual phrases
                        if ((line.Count() > 2) && (line[0] == 'V') && (line[1] == '_'))
                            continue;

                        // ignore lines without "Len"
                        if (!line.Contains("{Len", StringComparison.InvariantCultureIgnoreCase))
                            continue;

                        string[] line_parts = line.Split(" ");

                        if (line_parts.Count() > 1)
                        {
                            string name = line_parts[0];
                            string length = "";

                            int index = -1;
                            foreach (string part in line_parts)
                            {
                                ++index;

                                if (part.Equals("{Len"))
                                {
                                    length = (line_parts[index + 1]).Replace("}", "").Replace("closecaption", "");
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }

                            if (string.IsNullOrWhiteSpace(length))
                                continue;

                            int rowIndex = sentenceList.Rows.Add();
                            sentenceList.Rows[rowIndex].Cells["name"].Value = name;
                            sentenceList.Rows[rowIndex].Cells["len"].Value = length;
                        }
                        else
                        {
                            continue;
                        }
                    }
                }
            }
        }

        private void saveSentenceLen_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveDialog = new SaveFileDialog();
            saveDialog.FileName = "sentencelen.txt";
            saveDialog.Filter = "Text Files | *.txt";
            saveDialog.DefaultExt = "txt";
            DialogResult result = saveDialog.ShowDialog();
            if (result == DialogResult.OK)
            {
                KVObject parentObj = KVObject.ListCollection();

                foreach (DataGridViewRow row in sentenceList.Rows)
                {
                    string? szName = row.Cells["name"].Value.ToString();

                    if (szName == null)
                        continue;

                    string? szLen = row.Cells["len"].Value.ToString();

                    if (szLen == null)
                        continue;

                    parentObj.Add(szName, new KVObject(szLen));
                }

                string fileName = saveDialog.FileName;

                KVDocument doc = new KVDocument(null, "SentenceLen", parentObj);

                // Finally, save using a FileStream.
                KVSerializer kv = KVSerializer.Create(KVSerializationFormat.KeyValues1Text);
                using (FileStream stream = File.OpenWrite(fileName))
                {
                    byte[] info = new UTF8Encoding(true).GetBytes("// GENERATED WITH SENTENCELEN_GEN\n\n");
                    stream.Write(info, 0, info.Length);
                    kv.Serialize(stream, doc);
                }

                MessageBox.Show("sentencelen.txt generated!");
            }
        }
    }
}
