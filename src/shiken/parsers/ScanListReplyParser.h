#ifndef SHIKEN_SCAN_LIST_REPLY_PARSER_H
#define SHIKEN_SCAN_LIST_REPLY_PARSER_H

#include <shiken/config.h>
#include <boost/tuple/tuple.hpp> /* For boost::tie. */
#include <shiken/entities/Scan.h>
#include <shiken/utility/BarcodeProcessor.h>
#include "xsd_ScanListReply.h"

namespace shiken {

    class ScanListReplyParser: public xsd::shiken::ScanListReply_pskel {
    public:
        bool hasError() const {
            return m_tag == errorMessage_tag;
        }

        const QString &errorMessage() const {
            return m_errorMessage;
        }

        const QList<ExtendedScan> &scans() const {
            return m_scans;
        }

    public:
        virtual void pre() {
            m_tag = errorMessage_tag;
            m_errorMessage = QString();
            m_scans.clear();
        }

        virtual void choice_arm(choice_arm_tag tag) override {
            m_tag = tag;
        }

        virtual void errorMessage(const std::string &errorMessage) override {
            m_errorMessage = QString::fromUtf8(errorMessage.data());
        }

        virtual void scan(const ExtendedScan &scan) override {
            m_scans.push_back(scan);
        }

        virtual void post_ScanListReply() override {
            return;
        }

    private:
        choice_arm_tag m_tag;
        QString m_errorMessage;
        QList<ExtendedScan> m_scans; 
    };


    class ScanReplyParser: public xsd::shiken::Scan_pskel {
    public:
        virtual void pre() {
            m_scan = ExtendedScan();
        }

        virtual void scanId(int /*scanId*/) override {
            return;
        }

        virtual void size(long long /*size*/) override {
            return;
        }

        virtual void origName(const std::string &origName) override {
            m_scan.setFileName(QString::fromUtf8(origName.c_str()));
        }

        virtual void contentType(const std::string &/*contentType*/) override {
            return;
        }

        virtual void quizId(int /*quizId*/) override {
            return;
        }

        virtual void groupId(int /*groupId*/) override {
            return;
        }

        virtual void status(const std::string &status) override {
            Scan::State state;
            if(status == "RECOGNIZED")
                state = Scan::SERVER_RECOGNIZED;
            else if(status == "FAILED")
                state = Scan::SERVER_FAILED;
            else if(status == "WRONG")
                state = Scan::SERVER_WRONG;
            else if(status == "MANUAL")
                state = Scan::SERVER_MANUAL;
            else
                state = Scan::UPLOADED;
            m_scan.setState(state);
        }

        virtual void guidMode(int /*guidMode*/) override {
            return;
        }

        virtual void clientVersion(int /*clientVersion*/) override {
            return;
        }

        virtual void barcode(const std::string &barcode) override {
            QString compressedGuid;
            QDateTime printTime;
            boost::tie(compressedGuid, printTime) = BarcodeProcessor::decompose(QString::fromUtf8(barcode.c_str()));

            m_scan.setCompressedGuid(compressedGuid);
            m_scan.setPrintTime(printTime);
        }

        virtual void hashcode(const std::string &/*hashcode*/) override {
            return;
        }

        virtual void sha1(const std::string &sha1) override {
            m_scan.setHash(QString::fromUtf8(sha1.c_str()));
        }

        virtual void message(const std::string &/*message*/) override {
            return;
        }

        virtual void pageStamp(const std::string &/*pageStamp*/) override {
            return;
        }

        virtual void version(int /*version*/) override {
            return;
        }

        virtual void dpi(int /*dpi*/) override {
            return;
        }

        virtual void width(int /*width*/) override {
            return;
        }

        virtual void height(int /*height*/) override {
            return;
        }

        virtual void viewportX(int /*viewportX*/) override {
            return;
        }

        virtual void viewportY(int /*viewportY*/) override {
            return;
        }

        virtual void viewportW(int /*viewportW*/) override {
            return;
        }

        virtual void viewportH(int /*viewportH*/) override {
            return;
        }

        virtual void userId(int /*userId*/) override {
            return;
        }

        virtual ExtendedScan post_Scan() override {
            return m_scan;
        }

    private:
        ExtendedScan m_scan;
    };


} // namespace shiken

#endif // SHIKEN_SCAN_LIST_REPLY_PARSER_H
